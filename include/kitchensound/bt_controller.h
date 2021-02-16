#ifndef KITCHENSOUND_BT_CONTROLLER_H
#define KITCHENSOUND_BT_CONTROLLER_H

#include <functional>
#include <string>
#include <memory>

struct SDL_Thread;
class DBusBTController;
class FilePlayback;

class BTController {
public:
    explicit BTController(std::shared_ptr<FilePlayback>&);
    ~BTController();

    void activate_bt();
    void deactivate_bt();

    void set_metadata_status_callback(std::function<void(const std::string&, const std::string&)>);

private:
    friend class DBusBTController;

    void handle_update(const std::string&, const std::string&);
    std::function<void(const std::string&, const std::string&)> _cb_meta_status_update;

    std::shared_ptr<FilePlayback> _playback;
    std::unique_ptr<DBusBTController> _dbus;
    SDL_Thread *_thread;
};

#endif //KITCHENSOUND_BT_CONTROLLER_H
