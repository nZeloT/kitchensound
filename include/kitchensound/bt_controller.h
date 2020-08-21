#ifndef KITCHENSOUND_BT_CONTROLLER_H
#define KITCHENSOUND_BT_CONTROLLER_H

#include <functional>
#include <string>
#include <memory>

#include <SDL.h>

class DBusBTController;
class BTController {
public:
    explicit BTController(std::function<void(const std::string&, const std::string&)> handler);
    ~BTController();

    void activate_bt();
    void deactivate_bt();

private:
    friend class DBusBTController;

    void handle_update(const std::string&, const std::string&);
    std::function<void(const std::string&, const std::string&)> _handler;

    std::unique_ptr<DBusBTController> _dbus;
    SDL_Thread *_thread;
};

#endif //KITCHENSOUND_BT_CONTROLLER_H
