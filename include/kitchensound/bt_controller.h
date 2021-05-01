#ifndef KITCHENSOUND_BT_CONTROLLER_H
#define KITCHENSOUND_BT_CONTROLLER_H

#include <functional>
#include <string>
#include <memory>

#include <systemd/sd-bus.h>

class FdRegistry;
class AnalyticsLogger;
class FilePlayback;
struct Song;

class BTController {
public:
    BTController(std::unique_ptr<FdRegistry>&, std::unique_ptr<AnalyticsLogger>&, std::shared_ptr<FilePlayback>&);
    ~BTController();

    void activate_bt();
    void deactivate_bt();

    void set_metadata_status_callback(std::function<void(const std::string&, const Song&)>);

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;

    friend int dbus_signal_interface_added(sd_bus_message *, void* , sd_bus_error* );
    friend int dbus_signal_interface_removed(sd_bus_message *, void* , sd_bus_error* );
    friend int dbus_signal_properties_changed(sd_bus_message *, void *, sd_bus_error *);
};

#endif //KITCHENSOUND_BT_CONTROLLER_H
