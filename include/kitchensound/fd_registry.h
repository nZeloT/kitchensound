#ifndef KITCHENSOUND_FDREGISTRY_H
#define KITCHENSOUND_FDREGISTRY_H

#include <memory>
#include <functional>

class FdRegistry {
public:
    FdRegistry();

    ~FdRegistry();

    void addFd(int, std::function<void(int, uint32_t)>, uint32_t);

    void removeFd(int);

    void wait();

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

#endif //KITCHENSOUND_FDREGISTRY_H
