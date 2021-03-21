#include "kitchensound/bt_controller.h"

#include <memory>
#include <string>
#include <regex>
#include <unordered_map>
#include <cstdarg>
#include <sys/epoll.h>

#include <spdlog/spdlog.h>

#include "kitchensound/file_playback.h"
#include "kitchensound/fd_registry.h"
#include "kitchensound/timer.h"

#define BLUEZ_SERVICE_NAME "org.bluez"
#define DBUS_OBJECT_MANAGER_INTERFACE "org.freedesktop.DBus.ObjectManager"
#define DBUS_PROPERTIES_INTERFACE "org.freedesktop.DBus.Properties"

#define BLUEZ_DEVICE_INTERFACE "org.bluez.Device1"
#define BLUEZ_MEDIA_INTERFACE "org.bluez.MediaPlayer1"


struct BTDevice {
    std::string device_id;
    std::string device_name;
    std::string player_metadata;
    bool is_connected;
    bool has_player;
};

int dbus_signal_interface_added(sd_bus_message *, void *, sd_bus_error *);

int dbus_signal_interface_removed(sd_bus_message *, void *, sd_bus_error *);

int dbus_signal_properties_changed(sd_bus_message *, void *, sd_bus_error *);

struct BTController::Impl {
    Impl(std::unique_ptr<FdRegistry> &fdreg, std::shared_ptr<FilePlayback> &playback)
            : _cb_meta_status_update{[](std::string const &, std::string const &) {}}, _bus{nullptr}, _bus_fd{-1},
              _error{0}, _connected_device{nullptr},
              _known_devices{}, _playback{playback}, _fdreg{fdreg},
              _sdbus_update{std::make_unique<Timer>(fdreg, "BtController Dbus Update", 100, false, [this]() {
                  this->on_dbus_timer_update();
              })} {}

    ~Impl() {
        if (_bus)
            stop_signal_watchdog();
    }

    void change_adapter_state(const std::string &interface_member, bool turn_on) {
        sd_bus *bus = nullptr;
        _error = sd_bus_open_system(&bus);
        check_error("Failed to connect to system dbus!");

        _error = sd_bus_set_property(bus, "org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1",
                                     interface_member.c_str(),
                                     nullptr, "b", turn_on);
        check_error("Failed to change BT adapter power state!");

        SPDLOG_INFO("BT adapter property changed -> {} = {}", interface_member, turn_on);

        sd_bus_unref(bus);
    }

    void adapter_power_change(bool turn_on) {
        change_adapter_state("Powered", turn_on);
        change_adapter_state("Discoverable", turn_on);

        if (turn_on)
            start_signal_watchdog();
        else
            stop_signal_watchdog();

        _playback->playback(turn_on ? "bt-activate.mp3" : "bt-deactivate.mp3");
    }

    void start_signal_watchdog() {
        _error = sd_bus_open_system(&_bus);
        check_error("Failed to open system dbus!");

        //initially call the GetManagedObjects method once to identify all already known devices
        sd_bus_message *replies;
        _error = sd_bus_call_method(_bus, BLUEZ_SERVICE_NAME, "/", DBUS_OBJECT_MANAGER_INTERFACE, "GetManagedObjects",
                                    nullptr, &replies, nullptr);
        check_error("Failed to call DBus Object Manager!");

        _error = sd_bus_message_enter_container(replies, SD_BUS_TYPE_ARRAY, "{oa{sa{sv}}}");
        check_error("outermost array");

        while (!sd_bus_message_at_end(replies, false)) {
            _error = sd_bus_message_enter_container(replies, SD_BUS_TYPE_DICT_ENTRY, "oa{sa{sv}}");
            check_error("outermost dict enter");

            on_dbus_interface_added(replies);

            _error = sd_bus_message_exit_container(replies);
            check_error("failed to exit first dict");
        }


        _error = sd_bus_message_exit_container(replies);
        check_error("failed to exit ourtemost array");


        _error = sd_bus_match_signal(_bus, nullptr, BLUEZ_SERVICE_NAME, "/", DBUS_OBJECT_MANAGER_INTERFACE,
                                     "InterfacesAdded",
                                     dbus_signal_interface_added, this);
        check_error("failed to register for interfaces added signal");

        _error = sd_bus_match_signal(_bus, nullptr, BLUEZ_SERVICE_NAME, "/", DBUS_OBJECT_MANAGER_INTERFACE,
                                     "InterfacesRemoved",
                                     dbus_signal_interface_removed, this);
        check_error("failed to register for the interfaces removed signal");

        _bus_fd = sd_bus_get_fd(_bus);

        update_sd_bus_event_epoll();

        uint64_t microseconds;
        _error = sd_bus_get_timeout(_bus, &microseconds);
        if (_error < 0) {
            SPDLOG_WARN("Failed to obtain dbus timeout!, Defaulting to ten seconds!");
            microseconds = 10000000L;
        }

        _sdbus_update->reset_timer(std::max(1ull, microseconds / 1000L));
    };

    void stop_signal_watchdog() {
        _sdbus_update->stop();
        _fdreg->removeFd(_bus_fd);

        sd_bus_unref(_bus);
        _bus = nullptr;
        _connected_device = nullptr;
        _known_devices.clear();
    }

    void on_dbus_event_update(uint32_t _unused_) {
        update_dbus();
        update_sd_bus_event_epoll();
    }

    void on_dbus_timer_update() {
        update_dbus();
        update_sd_bus_event_epoll();
    }

    void update_dbus() {
        _error = 0;
        if (_bus) {
            for (;;) {
                //we're only interested in signals which in turn have callback handlers
                auto r = sd_bus_process(_bus, nullptr);
                if (r > 0)
                    continue;
                if (r == 0)
                    break;

                throw std::runtime_error{"Received error during sd_bus_process!"};
            }
        }
    }

    void update_sd_bus_event_epoll() {
        _fdreg->removeFd(_bus_fd);

        _bus_fd = sd_bus_get_fd(_bus);
        auto events = sd_bus_get_events(_bus);
        if (events < 0)
            throw std::runtime_error{"Failed to receive new dbus poll events!"};

        auto epoll_evts = ((events & POLL_IN) ? EPOLLIN : 0) | ((events & POLL_OUT) ? EPOLLOUT : 0);

        _fdreg->addFd(_bus_fd, [this](int fd, uint32_t revents) {
            this->on_dbus_event_update(revents);
        }, epoll_evts);
    }

    void on_dbus_interface_added(sd_bus_message *dbus_msg) {
        //expecting oa{sa{sv}}
        const char *path;
        _error = sd_bus_message_read(dbus_msg, "o", &path);
        check_error("dict path");


        //is path relevant for watching device states?
        if (std::string{path}.find("/dev_") == std::string::npos) {
            _error = sd_bus_message_skip(dbus_msg, "a{sa{sv}}");
            check_error("Failed to skip interface add on irrelevant path!");

            SPDLOG_INFO("Skipping interface added message for path -> {}", path);

            return;
        }

        // message is potentially relevant; check if the device is already known; if not make it known
        auto it = get_or_create_known_device(get_device_id_from_dbus_path(std::string{path}));

        _error = sd_bus_message_enter_container(dbus_msg, SD_BUS_TYPE_ARRAY, "{sa{sv}}");
        check_error("Failed to enter array of added interfaces");

        while (!sd_bus_message_at_end(dbus_msg, false)) {
            _error = sd_bus_message_enter_container(dbus_msg, SD_BUS_TYPE_DICT_ENTRY, "sa{sv}");
            check_error("Failed to enter dict entry of a added interface");

            const char *intf_name;
            _error = sd_bus_message_read(dbus_msg, "s", &intf_name);
            check_error("failed to read added interface name");


            //check whether it is a relevant interface
            std::string interface{intf_name};
            if (interface == BLUEZ_DEVICE_INTERFACE) {
                // a new device was registered; watch the device properties to change
                setup_new_device(it->second, std::string{path});
            } else if (interface == BLUEZ_MEDIA_INTERFACE) {
                // a new media player interface was added; this is normally only the case
                // when a connection to a phone is established
                // we want to register for changed properties here as well
                setup_new_player_on_device(it->second, std::string{path});
            }

            _error = sd_bus_message_skip(dbus_msg, "a{sv}");
            check_error("failed to skip");

            _error = sd_bus_message_exit_container(dbus_msg);
            check_error("failed to exit second dict");
        }

        _error = sd_bus_message_exit_container(dbus_msg);
        check_error("failed to exit second array");

    }

    void on_dbus_interface_removed(sd_bus_message *dbus_msg) {
        // expecting oas

        const char *path;
        _error = sd_bus_message_read(dbus_msg, "o", &path);
        check_error("Failed to read object path from interface removed message!");

        //check if path is relevant for us
        auto it = _known_devices.find(get_device_id_from_dbus_path(std::string{path}));
        if (it != std::end(_known_devices)) {
            //relevant path

            _error = sd_bus_message_enter_container(dbus_msg, SD_BUS_TYPE_ARRAY, "s");
            check_error("failed to enter container of strings");

            while (!sd_bus_message_at_end(dbus_msg, false)) {
                const char *intf_name;
                _error = sd_bus_message_read(dbus_msg, "s", &intf_name);
                check_error("failed to read removed interface name");

                std::string interface{intf_name};
                if (interface == BLUEZ_DEVICE_INTERFACE) {
                    //expect that the registered property listener is dead as well automatically
                    remove_device_from_known_devices(it);
                } else if (interface == BLUEZ_MEDIA_INTERFACE) {
                    remove_player_on_device(it->second, std::string{path});
                }
            }

            _error = sd_bus_message_exit_container(dbus_msg);
            check_error("Failed to exit array on interface rmoval");


        } else {
            _error = sd_bus_message_skip(dbus_msg, "as");
            check_error("failed to skip array of strings");
        }
    }

    void on_dbus_property_changed(sd_bus_message *dbus_msg) {
        //expecting message of kind "sa{sv}as

        // first string is of message is interface name
        // followed by an array of { property name -> variant with new state }

        // want to listen here for
        // 1. connected change on BLUEZ_DEVICE_INTERFACE
        // 2. alias change on same interface
        // 3. Track change on BLUEZ_MEDIA_INTERFACE

        auto p = sd_bus_message_get_path(dbus_msg);
        std::string path{p};

        if (path.find("/dev_") != std::string::npos) {
            auto it = _known_devices.find(get_device_id_from_dbus_path(path));
            if (it != std::end(_known_devices)) {
                const char *i;
                _error = sd_bus_message_read(dbus_msg, "s", &i);
                check_error("Failed to read interface name from property change message.");
                std::string interface{i};

                _error = sd_bus_message_enter_container(dbus_msg, SD_BUS_TYPE_ARRAY, "{sv}");
                check_error("Failed to open array on property change message.");

                if (interface == BLUEZ_DEVICE_INTERFACE) {
                    parse_device_property_change(it->second, dbus_msg);
                } else if (interface == BLUEZ_MEDIA_INTERFACE) {
                    parse_player_property_change(it->second, dbus_msg);
                } else {
                    SPDLOG_INFO("Skipping message on irrelevant interface for path -> {}; {}", path, interface);
                    while(!sd_bus_message_at_end(dbus_msg, false)) {
                        _error = sd_bus_message_skip(dbus_msg, "{sv}");
                        check_error("Failed to skip on irrelevant property changed interface");
                    }
                }

                _error = sd_bus_message_exit_container(dbus_msg);
                check_error("Failed to exit array on property change message");

                _error = sd_bus_message_skip(dbus_msg, "as");
                check_error("Failed to skip remaining array on property change message");
            } else {
                SPDLOG_INFO("Skipping message for unknown device on path -> {}", path);
            }
        } else {
            SPDLOG_INFO("Skipping message for irrelevant path -> {}", path);
        }
    }

    void setup_new_device(BTDevice &device, std::string const &path) {
        //1. read the connected, and alias properties for initial values
        //2. register for property changes
        _error = sd_bus_get_property_trivial(_bus, BLUEZ_SERVICE_NAME, path.c_str(), BLUEZ_DEVICE_INTERFACE,
                                             "Connected",
                                             nullptr, 'b', &device.is_connected);
        check_error("Failed to obtain connected property.");
        SPDLOG_DEBUG("Obtained connected state for device -> {}; {}", device.device_id, device.is_connected);

        char *device_name;
        _error = sd_bus_get_property_string(_bus, BLUEZ_SERVICE_NAME, path.c_str(), BLUEZ_DEVICE_INTERFACE, "Alias",
                                            nullptr, &device_name);
        check_error("Failed to obtain device name property.");
        device.device_name = std::string{device_name};
        SPDLOG_DEBUG("Obtained alias for device -> {}; {}", device.device_id, device.device_name);

        if (device.is_connected) {
            on_new_device_connected(device);
        }

        _error = sd_bus_match_signal(_bus, nullptr, BLUEZ_SERVICE_NAME, path.c_str(), DBUS_PROPERTIES_INTERFACE,
                                     "PropertiesChanged", dbus_signal_properties_changed, this);
        check_error("Failed to register for propertiy changes on the new device.");
    }

    void remove_device_from_known_devices(std::unordered_map<std::string, BTDevice>::iterator &it) {
        on_device_disconnected(it->second);
        _known_devices.erase(it);
    }

    void setup_new_player_on_device(BTDevice &device, std::string const &path) {
        // 1. read the current values for artist, track and album
        // 2. register for property changes
        sd_bus_message *dbus_msg;
        _error = sd_bus_get_property(_bus, BLUEZ_SERVICE_NAME, path.c_str(), BLUEZ_MEDIA_INTERFACE, "Track", nullptr,
                                     &dbus_msg, "a{sv}");
        if (_error < 0) {
            SPDLOG_WARN(
                    "Unable to fetch current Track from new Media Player. Probably has no Track property right now.");
        } else {
            //parse new track info
            parse_track_property_message(device, dbus_msg);
        }

        _error = sd_bus_match_signal(_bus, nullptr, BLUEZ_SERVICE_NAME, path.c_str(), DBUS_PROPERTIES_INTERFACE,
                                     "PropertiesChanged", dbus_signal_properties_changed, this);
        check_error("Failed to register on player property changes");

        device.has_player = true;

    }

    void remove_player_on_device(BTDevice &device, std::string const &path) const {
        device.has_player = false;
        device.player_metadata.clear();
        //as the player object is already removed expect that the signal handler is also already removed
        if (_connected_device == &device) {
            _cb_meta_status_update(device.device_name, device.player_metadata);
        }
    }

    static std::string get_device_id_from_dbus_path(std::string const &dbus_path) {
        //expecting a path like /org/bluez/hci0/dev_XX_XX_XX_XX_XX_XX [/...]
        SPDLOG_DEBUG("Trying to extract device id from path -> {}", dbus_path);
        std::smatch match;
        if (std::regex_search(dbus_path, match, std::regex{
                R"(dev_[ABCDEF0-9]{2}_[ABCDEF0-9]{2}_[ABCDEF0-9]{2}_[ABCDEF0-9]{2}_[ABCDEF0-9]{2}_[ABCDEF0-9]{2})"})) {
            return match[0];
        } else
            throw std::runtime_error{"Tried to extract device id from path without device id!"};
    }

    std::unordered_map<std::string, BTDevice>::iterator get_or_create_known_device(std::string const &device_id) {
        auto it = _known_devices.find(device_id);
        if (it == std::end(_known_devices)) {
            auto pair = _known_devices.emplace(device_id, BTDevice{device_id, "", "", false, false});
            if (!pair.second)
                throw std::runtime_error{"Failed to insert new devcie into known devcies map!"};
            it = pair.first;
        }
        return it;
    }

    void check_error(std::string const &msg) {
        if (_error < 0)
            throw std::runtime_error{msg};
        _error = 0;
    }

    void parse_track_property_message(BTDevice &device, sd_bus_message *dbus_msg) {
        std::string title;
        std::string album;
        std::string artist;

        //parsing Track info. expecting "a{sv}"
        _error = sd_bus_message_enter_container(dbus_msg, SD_BUS_TYPE_ARRAY, "{sv}");
        check_error("Failed to enter Track array");

        while (!sd_bus_message_at_end(dbus_msg, false)) {
            _error = sd_bus_message_enter_container(dbus_msg, SD_BUS_TYPE_DICT_ENTRY, "sv");
            check_error("Failed to enter Track dict");

            const char *prop_name;
            _error = sd_bus_message_read(dbus_msg, "s", &prop_name);
            check_error("Failed to read next property name from track dict");

            std::string property{prop_name};
            const char *t;
            if (property == "Title") {
                _error = sd_bus_message_read(dbus_msg, "v", "s", &t);
                check_error("Failed to read title variant.");
                title = std::string{t};
            } else if (property == "Artist") {
                _error = sd_bus_message_read(dbus_msg, "v", "s", &t);
                check_error("Faile to read artist variant.");
                artist = std::string{t};
            } else if (property == "Album") {
                _error = sd_bus_message_read(dbus_msg, "v", "s", &t);
                check_error("Failed to read alibum variant.");
                album = std::string{t};
            } else {
                _error = sd_bus_message_skip(dbus_msg, "v");
            }

            _error = sd_bus_message_exit_container(dbus_msg);
            check_error("Failed to exit Track dict");
        }

        _error = sd_bus_message_exit_container(dbus_msg);
        check_error("Failed to exit track array.");

        build_metadata_string(device, title, artist, album);
    }

    void build_metadata_string(BTDevice &device, std::string const &title, std::string const &artist,
                               std::string const &album) const {
        std::ostringstream s{};

        bool has_content = false;
        if (!title.empty()) {
            s << title;
            has_content = true;
        }

        if (!artist.empty()) {
            if (has_content)
                s << " - ";
            s << artist;
            has_content = true;
        }

        if (!album.empty()) {
            if (has_content)
                s << " (";
            s << album;
            if (has_content)
                s << ")";
        }

        device.player_metadata = s.str();
        if (_connected_device == &device) {
            _cb_meta_status_update(device.device_name, device.player_metadata);
        }
    }

    void parse_device_property_change(BTDevice &device, sd_bus_message *dbus_msg) {
        //expecting {sv} in an open array
        while (!sd_bus_message_at_end(dbus_msg, false)) {
            _error = sd_bus_message_enter_container(dbus_msg, SD_BUS_TYPE_DICT_ENTRY, "sv");
            check_error("Failed to open dict entry on device property changed.");

            const char *n;
            _error = sd_bus_message_read(dbus_msg, "s", &n);
            check_error("Failed to read property name on device proterty changed.");
            std::string name{n};

            //check for connected
            //check for changed alias
            if (name == "Connected") {
                bool c;
                _error = sd_bus_message_read(dbus_msg, "v", "b", &c);
                check_error("Failed to read boolean variant for connected device property");

                if (c != device.is_connected) {
                    device.is_connected = c;
                    if (device.is_connected) {
                        on_new_device_connected(device);
                    }else{
                        on_device_disconnected(device);
                    }
                }
            } else if (name == "Alias") {
                const char *a;
                _error = sd_bus_message_read(dbus_msg, "v", "s", &a);
                check_error("Failed to read string variant for aliad device property.");
                std::string alias{a};

                device.device_name = alias;
                if (_connected_device == &device)
                    _cb_meta_status_update(device.device_name, device.player_metadata);
            } else {
                SPDLOG_INFO("Skipping irrelevant device property -> {}", name);
                _error = sd_bus_message_skip(dbus_msg, "v");
                check_error("Failed to skip variant on irrelevant device property changed");
            }

            _error = sd_bus_message_exit_container(dbus_msg);
            check_error("Failed to exit dict on device property changed.");
        }
    }

    void parse_player_property_change(BTDevice &device, sd_bus_message *dbus_msg) {
        //expecting {av} in an open array
        while (!sd_bus_message_at_end(dbus_msg, false)) {
            _error = sd_bus_message_enter_container(dbus_msg, SD_BUS_TYPE_DICT_ENTRY, "sv");
            check_error("Failed to open dict entry on device property changed.");

            const char *n;
            _error = sd_bus_message_read(dbus_msg, "s", &n);
            check_error("Failed to read property name on device proterty changed.");
            std::string name{n};

            if (name == "Track") {
                _error = sd_bus_message_enter_container(dbus_msg, SD_BUS_TYPE_VARIANT, "a{sv}");
                check_error("Failed to enter variant for track player property change");

                parse_track_property_message(device, dbus_msg);

                _error = sd_bus_message_exit_container(dbus_msg);
                check_error("Failed to exit variant for track player property change");
            } else {
                SPDLOG_INFO("Skipping irrelevant player property change for device -> {}; {}", device.device_name,
                            name);
                _error = sd_bus_message_skip(dbus_msg, "v");
                check_error("Failed to skip irrlevant variant for player property change");
            }

            _error = sd_bus_message_exit_container(dbus_msg);
            check_error("Failed to exit dict on device property changed.");
        }
    }

    void on_new_device_connected(BTDevice& device) {
        if(_connected_device != &device){
            _connected_device = &device;
            _cb_meta_status_update(device.device_name, device.player_metadata);
            change_adapter_state("Discoverable", false);
            _playback->playback("bt-connect.mp3");
        }
    }

    void on_device_disconnected(BTDevice& device) {
        if(_connected_device == &device) {
            _connected_device = nullptr;
            _cb_meta_status_update("Not Connected", "");
            change_adapter_state("Discoverable", true);
            _playback->playback("bt-disconnect.mp3");
        }
    }

    std::function<void(const std::string &, const std::string &)> _cb_meta_status_update;

    sd_bus *_bus;
    int _bus_fd;
    int _error;

    BTDevice *_connected_device;

    std::unordered_map<std::string, BTDevice> _known_devices;

    std::shared_ptr<FilePlayback> _playback;
    std::unique_ptr<Timer> _sdbus_update;
    std::unique_ptr<FdRegistry> &_fdreg;
};

//Dbus signal callbacks just pointing back to the handlers within implementation
int dbus_signal_interface_added(sd_bus_message *message, void *payload, sd_bus_error *_unused_) {
    auto impl = reinterpret_cast<BTController::Impl *>(payload);
    impl->on_dbus_interface_added(message);
    return 0;
}

int dbus_signal_interface_removed(sd_bus_message *message, void *payload, sd_bus_error *_unused_) {
    auto impl = reinterpret_cast<BTController::Impl *>(payload);
    impl->on_dbus_interface_removed(message);
    return 0;
}

int dbus_signal_properties_changed(sd_bus_message *message, void *payload, sd_bus_error *_unused_) {
    auto impl = reinterpret_cast<BTController::Impl *>(payload);
    impl->on_dbus_property_changed(message);
    return 0;
}


BTController::BTController(std::unique_ptr<FdRegistry> &fdreg, std::shared_ptr<FilePlayback> &playback)
        : _impl{std::make_unique<Impl>(fdreg, playback)} {}

BTController::~BTController() = default;

void BTController::set_metadata_status_callback(std::function<void(const std::string &, const std::string &)> new_cb) {
    _impl->_cb_meta_status_update = std::move(new_cb);
}

void BTController::activate_bt() {
    _impl->adapter_power_change(true);
}

void BTController::deactivate_bt() {
    _impl->adapter_power_change(false);
}