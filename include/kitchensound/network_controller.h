#ifndef KITCHENSOUND_NETWORK_CONTROLLER_H
#define KITCHENSOUND_NETWORK_CONTROLLER_H

#include <memory>
#include <functional>

class FdRegistry;

enum HTTP_METHOD {
    GET,
    POST
};

struct Buffer {
    char* response;
    size_t size;
};

typedef void CURLM;
typedef void CURL;
typedef int curl_socket_t;

class NetworkController {
public:
    explicit NetworkController(std::unique_ptr<FdRegistry>&);
    ~NetworkController();

    void add_request(std::string const &, HTTP_METHOD,
                     std::function<void(std::string const &, bool, Buffer&)> callback, void * = nullptr,
                     size_t = 0);

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;

    friend int curl_multi_timer_cb(CURLM*, long, Impl*);
    friend int curl_socket_cb(CURL*, curl_socket_t, int, void*, void*);
    friend struct _Fetch;
};


#endif //KITCHENSOUND_NETWORK_CONTROLLER_H
