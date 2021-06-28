#include "kitchensound/network_controller.h"

#include <sys/epoll.h>

#include <curl/curl.h>
#include <curl/multi.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "kitchensound/fd_registry.h"
#include "kitchensound/timer.h"

//Implematation is a loose adaption from the curl ephipherfifo exmple here: https://curl.se/libcurl/c/ephiperfifo.html

#define curlm_case(x) \
    case x: s = std::string{__STRING(x)};

struct _Fetch {
    std::string url;
    Buffer response;
    char error[CURL_ERROR_SIZE];
    CURL *easy_handle;
    std::function<void(std::string const&, bool, Buffer&)> callback;
};

struct _SocketInfo {
    curl_socket_t sockfd;
};

int curl_multi_timer_cb(CURLM *, long request_timeout_ms, NetworkController::Impl *impl);

int curl_socket_cb(CURL *easy_handle, curl_socket_t socket, int what, void *cb_payload, void *socket_info);

std::size_t write_cb(void *ptr, std::size_t size, std::size_t nmemb, void *payload);

struct NetworkController::Impl {

    explicit Impl(std::unique_ptr<FdRegistry> &fdreg)
            : _fdreg{fdreg}, _curl_multi{nullptr}, _running_fetches{0},
            _curl_update{std::make_unique<Timer>(fdreg, "Network Manager Curl Update", 100, false, [this](){
                this->timer_wakeup();
            })}{
        _curl_multi = curl_multi_init();
        curl_multi_setopt(_curl_multi, CURLMOPT_SOCKETFUNCTION, curl_socket_cb);
        curl_multi_setopt(_curl_multi, CURLMOPT_SOCKETDATA, this);
        curl_multi_setopt(_curl_multi, CURLMOPT_TIMERFUNCTION, curl_multi_timer_cb);
        curl_multi_setopt(_curl_multi, CURLMOPT_TIMERDATA, this);
        _curl_update->reset_timer();
    }

    ~Impl() {
        SPDLOG_DEBUG("Dropping network manager ...");
        cleanup();
        SPDLOG_DEBUG("Network manager dropped");
    }

    void add_request(std::string const &url, HTTP_METHOD method,
                     std::function<void(std::string const &, bool, Buffer&)> callback, void *body,
                     size_t body_size) const {
        auto *fetch = reinterpret_cast<_Fetch *>(calloc(1, sizeof(_Fetch)));
        fetch->easy_handle = curl_easy_init();
        fetch->error[0] = '\0';
        if (!fetch->easy_handle) {
            throw std::runtime_error{"Failed to create curl easy handle!"};
        }
        fetch->url = std::string{url};
        fetch->callback = std::move(callback);

        curl_easy_setopt(fetch->easy_handle, CURLOPT_URL, url.c_str());

        if(method == HTTP_METHOD::POST) {
            if(body == nullptr)
                throw std::runtime_error{"Tried to add a POST request with body == nullptr!"};

            curl_easy_setopt(fetch->easy_handle, CURLOPT_POST, 1L);
            curl_easy_setopt(fetch->easy_handle, CURLOPT_POSTFIELDSIZE, body_size);
            curl_easy_setopt(fetch->easy_handle, CURLOPT_COPYPOSTFIELDS, body);
        }

        curl_easy_setopt(fetch->easy_handle, CURLOPT_WRITEDATA, &fetch->response);
        curl_easy_setopt(fetch->easy_handle, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(fetch->easy_handle, CURLOPT_ERRORBUFFER, fetch->error);
        curl_easy_setopt(fetch->easy_handle, CURLOPT_PRIVATE, fetch);
        curl_easy_setopt(fetch->easy_handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(fetch->easy_handle, CURLOPT_LOW_SPEED_TIME, 3L);
        curl_easy_setopt(fetch->easy_handle, CURLOPT_LOW_SPEED_LIMIT, 10L);

        SPDLOG_INFO("Adding to curl multi -> {0}", url);
        auto rc = curl_multi_add_handle(_curl_multi, fetch->easy_handle);
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
        change_socket(socketp, easy_handle, socket, what);
        curl_multi_assign(_curl_multi, socket, socketp);
    }

    void change_socket(_SocketInfo* socketp, CURL* easy_handle, curl_socket_t socket, int what) {
        auto kind = ((what & CURL_POLL_IN) ? EPOLLIN : 0) | ((what & CURL_POLL_OUT) ? EPOLLOUT : 0);

        if(socketp->sockfd){
            _fdreg->removeFd(socketp->sockfd);
        }

        socketp->sockfd = socket;

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

    void check_multi_info() const {
        CURLMsg* msg;
        int msg_left;
        _Fetch* fetch;
        CURLcode res;

        while((msg = curl_multi_info_read(_curl_multi, &msg_left))) {
            if(msg->msg == CURLMSG_DONE) {
                res = msg->data.result;
                long status_code = -1;
                curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &fetch);
                curl_easy_getinfo(msg->easy_handle, CURLINFO_HTTP_CODE, &status_code);
                SPDLOG_INFO("Completed fetch -> {0}; {1}; {2};", fetch->url, status_code, fetch->error);

                fetch->callback(fetch->url, res == CURLE_OK && status_code < 400, fetch->response);

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

    void cleanup() {
        curl_multi_cleanup(_curl_multi);
        _curl_multi = nullptr;
    }

    int _running_fetches;
    std::unique_ptr<Timer> _curl_update;
    CURLM *_curl_multi;
    std::unique_ptr<FdRegistry> &_fdreg;
};

int curl_multi_timer_cb(CURLM*, long request_timeout_ms, NetworkController::Impl* impl) {
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
    auto impl = reinterpret_cast<NetworkController::Impl*>(cb_payload);
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

size_t write_cb(void *data, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    auto mem = reinterpret_cast<Buffer*>(userp);

    auto ptr = reinterpret_cast<char*>(realloc(mem->response, mem->size + realsize + 1));
    if(ptr == nullptr)
        return 0;  /* out of memory! */

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;

    return realsize;
}


NetworkController::NetworkController(std::unique_ptr<FdRegistry> &fdreg)
        : _impl{std::make_unique<Impl>(fdreg)} {}

NetworkController::~NetworkController() {
    SPDLOG_DEBUG("Network Controller dropped.");
}

void
NetworkController::add_request(std::string const &url, HTTP_METHOD method,
                               std::function<void(std::string const &, bool, Buffer&)> callback, void *body,
                               size_t body_size) {
    _impl->add_request(url, method, std::move(callback), body, body_size);
}