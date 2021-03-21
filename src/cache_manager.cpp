#include "kitchensound/cache_manager.h"

#include <fstream>
#include <sys/epoll.h>

#include <spdlog/spdlog.h>
#include <curl/curl.h>

#include "kitchensound/resource_manager.h"
#include "kitchensound/fd_registry.h"
#include "kitchensound/timer.h"

//Implematation is a loose adaption from the curl ephipherfifo exmple here: https://curl.se/libcurl/c/ephiperfifo.html

#define curlm_case(x) \
    case x: s = std::string{__STRING(x)};

struct _Fetch {
    std::string cache_id;
    std::string url;
    FILE* cache_file;
    char error[CURL_ERROR_SIZE];
    CURL* easy_handle;
    CacheManager::Impl* _origin;
};

struct _SocketInfo {
    curl_socket_t sockfd;
    CURL* easy_handle;
    int action;
    long timeout;
    CacheManager::Impl* _origin;
};


int curl_multi_timer_cb(CURLM*, long request_timeout_ms, CacheManager::Impl* impl);
int curl_socket_cb(CURL* easy_handle, curl_socket_t socket, int what, void* cb_payload, void* socket_info);
std::size_t write_cb(void* ptr, std::size_t size, std::size_t nmemb, void* payload);


struct CacheManager::Impl {

    Impl(std::unique_ptr<FdRegistry>& fdreg, ResourceManager &res, std::filesystem::path &cache_root)
            : _res{res}, _cache_root{cache_root}, _curl_multi{nullptr}, _fdreg{fdreg}, _running_fetches{0},
            _curl_update{std::make_unique<Timer>(fdreg, "Cache Manager Curl Update", 100, false, [this](){
                this->timer_wakeup();
            })} {
        std::filesystem::create_directory(cache_root);

        _curl_multi = curl_multi_init();
        curl_multi_setopt(_curl_multi, CURLMOPT_SOCKETFUNCTION, curl_socket_cb);
        curl_multi_setopt(_curl_multi, CURLMOPT_SOCKETDATA, this);
        curl_multi_setopt(_curl_multi, CURLMOPT_TIMERFUNCTION, curl_multi_timer_cb);
        curl_multi_setopt(_curl_multi, CURLMOPT_TIMERDATA, this);
        _curl_update->reset_timer();
    };

    ~Impl(){
        cleanup();
    }

    void load_from_cache(const std::string& url) {
        //check if already fetched, if not fetch it
        auto url_hash = std::hash<std::string>{}(url);
        auto cache_id = std::string{_cache_root.string() + std::to_string(url_hash)};

        if(std::filesystem::exists(std::filesystem::path{cache_id})){
            SPDLOG_DEBUG("Requested ressource found in cache -> {}; {}", cache_id, url);
            load_success(url, cache_id);
        }else{
            schedule_for_fetch(cache_id, url);
        }

    }

    void schedule_for_fetch(std::string const& cache_id, const std::string &url) {

        _Fetch* fetch;
        CURLMcode rc;

        fetch = reinterpret_cast<_Fetch*>(calloc(1, sizeof(_Fetch)));
        fetch->easy_handle = curl_easy_init();
        fetch->error[0] = '\0';
        if(!fetch->easy_handle){
            throw std::runtime_error{"Failed to create curl easy handle!"};
        }
        fetch->_origin = this;
        fetch->url = std::string{url};
        fetch->cache_id = std::string{cache_id};
        fetch->cache_file = fopen(cache_id.c_str(), "w");
        curl_easy_setopt(fetch->easy_handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(fetch->easy_handle, CURLOPT_WRITEDATA, fetch->cache_file);
        curl_easy_setopt(fetch->easy_handle, CURLOPT_ERRORBUFFER, fetch->error);
        curl_easy_setopt(fetch->easy_handle, CURLOPT_PRIVATE, fetch);
        curl_easy_setopt(fetch->easy_handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(fetch->easy_handle, CURLOPT_LOW_SPEED_TIME, 3L);
        curl_easy_setopt(fetch->easy_handle, CURLOPT_LOW_SPEED_LIMIT, 10L);

        SPDLOG_INFO("Adding to fetch queue -> {0} :: {1}", cache_id, url);
        rc = curl_multi_add_handle(_curl_multi, fetch->easy_handle);
        check_mcode_die_on_fail(rc);
    }

    void timer_wakeup() {
        auto rc = curl_multi_socket_action(_curl_multi, CURL_SOCKET_TIMEOUT, 0, &_running_fetches);
        check_mcode_die_on_fail(rc);
        check_multi_info();
    }

    void add_socket(CURL* easy_handle, curl_socket_t socket, int what) {
        auto socketp = reinterpret_cast<_SocketInfo*>(calloc(1, sizeof(_SocketInfo)));
        socketp->sockfd = socket;
        socketp->_origin = this;
        change_socket(socketp, easy_handle, socket, what);
        curl_multi_assign(_curl_multi, socket, socketp);
    }

    void change_socket(_SocketInfo* socketp, CURL* easy_handle, curl_socket_t socket, int what) {
        auto kind = ((what & CURL_POLL_IN) ? EPOLLIN : 0) | ((what & CURL_POLL_OUT) ? EPOLLOUT : 0);

        if(socketp->sockfd){
            _fdreg->removeFd(socketp->sockfd);
        }

        socketp->sockfd = socket;
        socketp->easy_handle = easy_handle;
        socketp->action = what;

        _fdreg->addFd(socket, [this](int fd, uint32_t events){
            this->handle_socket_event(fd, events);
        }, kind);
    }

    void remove_socket(_SocketInfo* socketp) const {
        if(socketp != nullptr) {
            if(socketp->sockfd) {
                _fdreg->removeFd(socketp->sockfd);
            }
            free(socketp);
        }
    }

    void handle_socket_event(int fd, uint32_t events) {
        auto action = ((events & EPOLLIN) ? CURL_CSELECT_IN : 0) | ((events & EPOLLOUT) ? CURL_CSELECT_OUT : 0);
        auto rc = curl_multi_socket_action(_curl_multi, fd, action, &_running_fetches);
        check_mcode_die_on_fail(rc);

        check_multi_info();
        if(_running_fetches <= 0){
            //finished last tranfer
            SPDLOG_DEBUG("Curl signals finished last transfer, so stopping update timer.");
            _curl_update->stop();
        }
    }

    void check_multi_info() {
        CURLMsg* msg;
        int msg_left;
        _Fetch* fetch;
        CURLcode res;

        while((msg = curl_multi_info_read(_curl_multi, &msg_left))) {
            if(msg->msg == CURLMSG_DONE) {
                res = msg->data.result;
                curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &fetch);
                SPDLOG_INFO("Completed fetch -> {0}; {1}; {2};", fetch->url, res, fetch->error);
                fflush(fetch->cache_file);
                fclose(fetch->cache_file);

                if(res == CURLE_OK)
                    load_success(fetch->url, fetch->cache_id);
                else
                    load_failed(fetch->url);

                curl_multi_remove_handle(_curl_multi, msg->easy_handle);
                curl_easy_cleanup(fetch->easy_handle);
                free(fetch);
            }
        }
    }

    static void check_mcode_die_on_fail(CURLMcode code) {
        if(code != CURLM_OK) {
            std::string s;
            switch (code) {
                curlm_case(CURLM_BAD_HANDLE) break;
                curlm_case(CURLM_BAD_EASY_HANDLE) break;
                curlm_case(CURLM_OUT_OF_MEMORY) break;
                curlm_case(CURLM_INTERNAL_ERROR) break;
                curlm_case(CURLM_LAST) break;
                default: s = std::string{"Unknown CURLM error!"};
                curlm_case(CURLM_BAD_SOCKET)
                    SPDLOG_INFO("Received CURLM error -> {}", s);
                return; //continue after having a bad socket
            }

            throw std::runtime_error{"Raised CURLM error -> " + s};
        }
    }

    void load_failed(const std::string &url) {
        SPDLOG_DEBUG("Notifying Resourcemanager about failed load -> {}", url);
        _res.cache_load_failed(url);
    }

    void load_success(const std::string &url, const std::string &cache_id) {
        SPDLOG_DEBUG("Notifying Resourcemanager about successful load -> {}", url);
        _res.cache_load_success(url, ResourceManager::load_image_raw(cache_id));
    }

    void cleanup() {
        curl_multi_cleanup(_curl_multi);
        _curl_multi = nullptr;
    }


    int _running_fetches;
    CURLM* _curl_multi;
    std::unique_ptr<Timer> _curl_update;

    std::unique_ptr<FdRegistry>& _fdreg;
    ResourceManager &_res;
    std::filesystem::path _cache_root;
};


int curl_multi_timer_cb(CURLM*, long request_timeout_ms, CacheManager::Impl* impl) {
    SPDLOG_DEBUG("Resetting timer to -> {}ms", request_timeout_ms);
    if(request_timeout_ms > 0){
        impl->_curl_update->reset_timer(request_timeout_ms);
    }else if(request_timeout_ms == 0){
        impl->_curl_update->reset_timer(1);
    }else{
        SPDLOG_DEBUG("Received multi timer request < 0ms so stopping update timer.");
        impl->_curl_update->stop();
    }
    return 0;
}

int curl_socket_cb(CURL* easy_handle, curl_socket_t socket, int what, void* cb_payload, void* socket_info) {
    auto impl = reinterpret_cast<CacheManager::Impl*>(cb_payload);
    auto socketp = reinterpret_cast<_SocketInfo*>(socket_info);
    if(what == CURL_POLL_REMOVE){
        impl->remove_socket(socketp);
    }else{
        if(socketp == nullptr) {
            impl->add_socket(easy_handle, socket, what);
        }else{
            impl->change_socket(socketp, easy_handle, socket, what);
        }
    }

    return 0;
}

CacheManager::CacheManager(std::unique_ptr<FdRegistry>& fdreg, ResourceManager &res, std::filesystem::path &cache_root)
        : _impl{std::make_unique<Impl>(fdreg, res, cache_root)} {}

CacheManager::~CacheManager() = default;

void CacheManager::load_from_cache(const std::string &url) {
    _impl->load_from_cache(url);
}