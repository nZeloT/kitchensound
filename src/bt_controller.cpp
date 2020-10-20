#include "kitchensound/bt_controller.h"

#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <string>
#include <map>
#include <sstream>


#include <spdlog/spdlog.h>
#include <sdbus-c++/sdbus-c++.h>

constexpr char SERVICE_NAME[] = "org.bluez";

//taken from SO: https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-the-output-of-the-command-within-c-using-po/479103#479103
// credit to @waqas + @gregpaton
static std::string exec(const std::string &cmd) {
    std::array<char, 128> buffer{};
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
        throw std::runtime_error("Failed to execute command!");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        result += buffer.data();
    return result;
}

class DBusBTDeviceMonitor {

public:
    DBusBTDeviceMonitor(sdbus::IConnection &conn, const std::string &deviceId,
                        std::function<void(std::string const &, bool)> deviceConnectedHandler,
                        std::function<void(std::string const &, sdbus::ObjectPath const &)> devicePlayerChange) :
            _proxy{std::move(sdbus::createProxy(conn, SERVICE_NAME, "/org/bluez/hci0/" + deviceId))},
            _deviceConnectedHandler(std::move(deviceConnectedHandler)),
            _devicePlayerChanged(std::move(devicePlayerChange)),
            _deviceId(deviceId) {

        //0. check current device _state
        auto prop = _proxy->getProperty("Connected").onInterface(DEVICE_INTERFACE);
        _connected = prop.get<bool>();
        spdlog::info("Initial device state: {0}; {1}", _deviceId, _connected);
        _deviceConnectedHandler(_deviceId, _connected);

        try {
            prop = _proxy->getProperty("Player").onInterface(MEDIAPLAYER_INTF);
            _playerPath = prop.get<sdbus::ObjectPath>();
            _devicePlayerChanged(_deviceId, _playerPath);
        } catch (sdbus::Error const &error) {
            spdlog::error(error.getMessage());
        }

        //1. register change handler
        _proxy->uponSignal("PropertiesChanged").onInterface(INTERFACE_NAME)
                .call([this](const std::string &interfaceName,
                             const std::map<std::string, sdbus::Variant> &changedProperties,
                             const std::vector<std::string> &invalidatedProperties) {
                    this->on_properties_changed(interfaceName, changedProperties, invalidatedProperties);
                });

        _proxy->finishRegistration();
    }

    ~DBusBTDeviceMonitor() = default;

    [[nodiscard]] std::string getDeviceId() { return _deviceId; };

private:
    static constexpr char INTERFACE_NAME[] = "org.freedesktop.DBus.Properties";
    static constexpr char DEVICE_INTERFACE[] = "org.bluez.Device1";
    static constexpr char MEDIAPLAYER_INTF[] = "org.bluez.MediaControl1";

    void on_properties_changed(const std::string &interfaceName,
                               const std::map<std::string, sdbus::Variant> &changedProperties,
                               const std::vector<std::string> &invalidatedProperties) {
        spdlog::info("DBusBTDeviceMonitor::on_properties_changed(): {0}", interfaceName);
        if (interfaceName == DEVICE_INTERFACE) {
            check_connected_property(changedProperties);
        } else if (interfaceName == MEDIAPLAYER_INTF) {
            check_media_player_property(changedProperties);
        }
    }

    void check_connected_property(std::map<std::string, sdbus::Variant> const &props) {
        auto val = props.find("Connected");
        if (val != props.end()) {
            _connected = val->second.get<bool>();
            spdlog::info("DBusBTDeviceMonitor::check_connected_property(): New Device status: {0}; {1}", _deviceId,
                         _connected);
            _deviceConnectedHandler(_deviceId, _connected);
        }
    }

    void check_media_player_property(std::map<std::string, sdbus::Variant> const &props) {
        auto val = props.find("Player");
        if (val != props.end()) {
            _playerPath = val->second.get<sdbus::ObjectPath>();
            spdlog::info("DBusBTDeviceMonitor::check_media_player_property(): Player Property changed: {0}; {1}",
                         _deviceId, _playerPath);
            _devicePlayerChanged(_deviceId, _playerPath);
        }
    }

    bool _connected;
    sdbus::ObjectPath _playerPath;

    std::string _deviceId;
    std::unique_ptr<sdbus::IProxy> _proxy;
    std::function<void(std::string const &, bool)> _deviceConnectedHandler;
    std::function<void(std::string const &, std::string const &)> _devicePlayerChanged;
};

class DBusBTObjectMonitor {
public:
    DBusBTObjectMonitor(sdbus::IConnection &conn,
                        std::function<void(std::string const &, bool)> deviceChanged)
            : _proxy{std::move(sdbus::createProxy(conn, SERVICE_NAME, "/"))},
              _deviceChange(std::move(deviceChanged)),
              _known() {

        //0. check for existing devices and players
        std::map<sdbus::ObjectPath, std::map<std::string, std::map<std::string, sdbus::Variant>>> objects;
        _proxy->callMethod("GetManagedObjects").onInterface(INTERFACE_NAME).storeResultsTo(objects);
        for (auto &entry : objects) {
            this->on_interfaces_added(entry.first);
        }

        //1. register for change events
        _proxy->uponSignal("InterfacesAdded")
                .onInterface(INTERFACE_NAME)
                .call([this](const sdbus::ObjectPath &objectPath,
                             const std::map<std::string, std::map<std::string, sdbus::Variant>> &interfacesAndProperties) {
                    this->on_interfaces_added(objectPath);
                });

        _proxy->uponSignal("InterfacesRemoved")
                .onInterface(INTERFACE_NAME)
                .call([this](const sdbus::ObjectPath &objectPath, const std::vector<std::string> &interfaces) {
                    this->on_interfaces_removed(objectPath);
                });

        _proxy->finishRegistration();
    }

    ~DBusBTObjectMonitor() = default;

private:
    static constexpr const char INTERFACE_NAME[] = "org.freedesktop.DBus.ObjectManager";

    void on_interfaces_added(const sdbus::ObjectPath &objectPath) {
        //check if its a device
        int pos;
        if ((pos = objectPath.find("/dev_")) != std::string::npos) {
            //is it a known device?
            spdlog::info("DBusBTObjectMonitor::on_interfaces_added(): {0}", objectPath);
            int len = objectPath.find_first_of("/", pos + 1);
            if (len != std::string::npos) {
                len -= pos;
                --len;
            }
            auto deviceId = objectPath.substr(pos + 1, len);
            spdlog::info("DBusBTObjectMonitor::on_interfaces_added(): Found device Id: {0}", deviceId);

            //is the device id already known?
            auto known = std::find(begin(_known), end(_known), deviceId);
            if (known == std::end(_known)) {
                spdlog::info("DBusBTObjectMonitor::on_interfaces_added(): Is formerly unknown device");
                _known.push_back(deviceId);
                _deviceChange(deviceId, true);
            }
        }
    }

    void on_interfaces_removed(const sdbus::ObjectPath &objectPath) {
        int devPos;
        if ((devPos = objectPath.find("/dev")) != std::string::npos) {
            int len = objectPath.find_first_of("/", devPos + 1);
            if (len != std::string::npos) {
                spdlog::info("DBusBTObjectMonitor::on_interfaces_removed(): Not the device itself is removed");
                return; //its not the device beeing removed ...
            }

            auto deviceId = objectPath.substr(devPos + 1, len);
            spdlog::info("DBusBTObjectMonitor::on_interfaces_removed(): Found device Id: {0}", deviceId);

            auto known = std::find(begin(_known), end(_known), deviceId);
            if (known == std::end(_known))
                return; //removing unknown device ...

            spdlog::info("DBusBTObjectMonitor::on_interfaces_removed(): removing device {0}", deviceId);
            _known.erase(known);
        }
    }


    std::vector<std::string> _known;
    std::function<void(std::string const &, bool)> _deviceChange;
    std::unique_ptr<sdbus::IProxy> _proxy;
};

class DBusBTMediaPlayerMonitor {
public:
    DBusBTMediaPlayerMonitor(sdbus::IConnection &conn, std::string const &deviceId, sdbus::ObjectPath const &playerId,
                             std::function<void(std::string)> newMetadata, bool enabled = false)
            : _proxy{std::move(sdbus::createProxy(conn, SERVICE_NAME, playerId))},
              _newMetadata(std::move(newMetadata)), _metadata(), _enabled(enabled), _player(playerId) {

        //0. check existing metadata; Track property is optional and my not yet be present
        try {
            std::map<std::string, sdbus::Variant> props;
            auto _p = _proxy->getProperty("Track").onInterface(MEDIAPLAYER_INTF);
            auto metadata = _p.get<std::map<std::string, sdbus::Variant>>();

            //potentially has Title, Artist, Album
            build_metadata_string(metadata);
            spdlog::info("DBusBTMediaPlayerMonitor::C-Tor(): {0}", _metadata);
        } catch (sdbus::Error &error) {
            spdlog::error("DBusBTMediaPlayerMonitor::C-Tor(): {0}", error.getMessage());
        }

        if (_enabled)
            _newMetadata(_metadata);

        //1. register for change event
        _proxy->uponSignal("PropertiesChanged").onInterface(INTERFACE_NAME)
                .call([this](const std::string &interfaceName,
                             const std::map<std::string, sdbus::Variant> &changedProperties,
                             const std::vector<std::string> &invalidatedProperties) {
                    this->on_properties_changed(interfaceName, changedProperties);
                });

        _proxy->finishRegistration();
    }

    void set_enabled(bool enabled) {
        spdlog::info("DBusBTMediaPlayerMonitor::set_enabled(): {0} : {1}", _player, enabled);
        _enabled = enabled;
    }

private:
    static constexpr char INTERFACE_NAME[] = "org.freedesktop.DBus.Properties";
    static constexpr char MEDIAPLAYER_INTF[] = "org.bluez.MediaPlayer1";

    void on_properties_changed(std::string const &intf, std::map<std::string, sdbus::Variant> const &props) {
        spdlog::info("DBusBTMediaPlayerMonitor::on_properties_changed(): {0}", intf);
        if (intf == MEDIAPLAYER_INTF) {
            //try to find the track property
            auto it = std::find_if(begin(props), end(props), [this](const auto &p) {
                return p.first == "Track";
            });
            if (it != end(props))
                build_metadata_string(it->second.get<std::map<std::string, sdbus::Variant>>());
            if (_enabled)
                _newMetadata(_metadata);
        }
    }

    void build_metadata_string(std::map<std::string, sdbus::Variant> const &meta) {
        std::ostringstream s{};
        auto it = meta.find("Title");
        bool hasContent = false;
        if (it != std::end(meta)) {
            auto t = it->second.get<std::string>();
            if(!t.empty())
                hasContent = true;
            s << t;
        }

        it = meta.find("Artist");
        if (it != std::end(meta)) {
            if (hasContent)
                s << " - ";
            auto t = it->second.get<std::string>();
            if(!t.empty())
                hasContent = true;
            s << t;
        }

        it = meta.find("Album");
        if (it != std::end(meta)) {
            auto t = it->second.get<std::string>();
            if (hasContent && !t.empty())
                s << " (";
            s << t;
            if (hasContent && !t.empty())
                s << ")";
        }

        _metadata = s.str();
        spdlog::info("DBusBTMediaPlayerMonitor::build_metadata_string(): {0}", _metadata);
    }

    bool _enabled;
    std::string _metadata;

    std::string _player;
    std::unique_ptr<sdbus::IProxy> _proxy;
    std::function<void(std::string)> _newMetadata;
};

class DBusBTController {
public:

    explicit DBusBTController(std::function<void(const std::string &, const std::string &)> handler)
            : _handler(std::move(handler)),
              _dbusConnection(sdbus::createSystemBusConnection()),
              _objectMonitor{}, _deviceMonitors{}, _playerMonitors{}, _connected(true),
              _connectedDevice{}, _status{}, _metadata{} {

        spdlog::info("DBusBTController::C-Tor(): crteated connection; creating DBusBTObjectMonitor");
        _objectMonitor = std::make_unique<DBusBTObjectMonitor>(*_dbusConnection,
                                                               [this](const std::string &device, bool added) {
                                                                   if (added) {
                                                                       add_device_monitor(device);
                                                                   } else {
                                                                       remove_device_monitor(device);
                                                                   }
                                                               });
    }

    ~DBusBTController() = default;

    std::unique_ptr<sdbus::IConnection> _dbusConnection;

private:
    void add_device_monitor(std::string const &deviceId) {
        _deviceMonitors.emplace(_deviceMonitors.begin(),
                                std::make_unique<DBusBTDeviceMonitor>(*_dbusConnection, std::string{deviceId},
                                                                      [this](std::string const &deviceId,
                                                                             bool connected) {
                                                                          if (connected)
                                                                              device_connected(deviceId);
                                                                          else
                                                                              device_disconnected(deviceId);
                                                                      },
                                                                      [this](std::string const &deviceId,
                                                                             sdbus::ObjectPath const &playerPath) {
                                                                          player_changed(deviceId, playerPath);
                                                                      }));
    }

    void remove_device_monitor(std::string const &deviceId) {
        auto it = std::find_if(begin(_deviceMonitors), end(_deviceMonitors), [&deviceId](auto const &p) {
            return p->getDeviceId() == deviceId;
        });
        if (it != std::end(_deviceMonitors))
            _deviceMonitors.erase(it);

        auto it2 = std::find_if(begin(_playerMonitors), end(_playerMonitors),
                                [&deviceId](const auto &it) -> bool {
                                    return it.first == deviceId;
                                });
        if (it2 != end(_playerMonitors))
            _playerMonitors.erase(it2);
    }

    void device_connected(std::string const &deviceId) {
        if (_connected)
            throw std::runtime_error("Still connected to a device; Can't be connected to more than one devices!");
        _connected = true;
        _connectedDevice = std::string{deviceId};
        _status = "Connected to " + deviceId;
        _metadata = "";
        _handler(_status, _metadata);

        auto it = std::find_if(begin(_playerMonitors), end(_playerMonitors), [&deviceId](const auto &it) {
            return it.first == deviceId;
        });
        if (it != std::end(_playerMonitors))
            it->second->set_enabled(true);
    }

    void device_disconnected(std::string const &deviceId) {
        //Not sure about the part below; In theory multiple new devices could be discovered simultaneously while
        //none of them is connected; While one is connected device discovery is shut off. Therefore, during initialization
        //multiple calls to this are possible.
        //if (!_connected)
        //    throw std::runtime_error("Wasn't connected to a device; Can't disconnect");

        auto it = std::find_if(begin(_playerMonitors), end(_playerMonitors), [&deviceId](const auto &it) {
            return it.first == deviceId;
        });
        if (it != std::end(_playerMonitors))
            it->second->set_enabled(false);

        _connected = false;
        _connectedDevice = "";
        _status = "Not Connected.";
        _metadata = "";
        _handler(_status, _metadata);
    }

    void player_changed(std::string const &deviceId, sdbus::ObjectPath const &playerPath) {
        auto it = std::find_if(begin(_playerMonitors), end(_playerMonitors),
                               [&deviceId](const auto &it) {
                                   return it.first == deviceId;
                               });
        if (it != std::end(_playerMonitors))
            _playerMonitors.erase(it);

        _playerMonitors.emplace(std::string{deviceId},
                                std::make_unique<DBusBTMediaPlayerMonitor>(*_dbusConnection, deviceId, playerPath,
                                                                           [this](std::string const &meta) {
                                                                               new_metadata(meta);
                                                                           }, deviceId == _connectedDevice));
    }

    void new_metadata(std::string const &metadata) {
        _metadata = std::string{metadata};
        _handler(_status, _metadata);
    }

    bool _connected;
    std::string _connectedDevice;
    std::string _status;
    std::string _metadata;

    std::function<void(const std::string &, const std::string &)> _handler;

    std::map<std::string, std::unique_ptr<DBusBTMediaPlayerMonitor>> _playerMonitors;
    std::vector<std::unique_ptr<DBusBTDeviceMonitor>> _deviceMonitors;
    std::unique_ptr<DBusBTObjectMonitor> _objectMonitor;
};

static int run_dbusbtconnector(void *inst_ptr) {
    auto instance = reinterpret_cast<DBusBTController *>(inst_ptr);
    instance->_dbusConnection->enterEventLoop();
    return 0;
}

BTController::BTController(std::function<void(const std::string &, const std::string &)> handler)
        : _handler{std::move(handler)},
          _dbus{std::make_unique<DBusBTController>([this](auto status, auto meta) {
              this->handle_update(status, meta);
          })}, _thread{nullptr} {}

BTController::~BTController() = default;

void BTController::handle_update(const std::string &status, const std::string &meta) {
    _handler(status, meta);
}

void BTController::activate_bt() {
    auto res = exec("../res/scripts/bt_on.sh");
    if (!res.find("succeeded"))
        throw std::runtime_error("Failed to activate BT!");
    _thread = SDL_CreateThread(::run_dbusbtconnector, "DbusBTController", reinterpret_cast<void *>(_dbus.get()));
    exec("aplay -Dvolumedev /usr/local/share/sounds/bt-activate.wav");
}

void BTController::deactivate_bt() {
    _dbus->_dbusConnection->leaveEventLoop();
    int status;
    SDL_WaitThread(_thread, &status);
    _thread = nullptr;
    auto res = exec("../res/scripts/bt_off.sh");
    if (!res.find("succeeded"))
        throw std::runtime_error("Failed to deactivate BT!");
    exec("aplay -D volumedev /usr/local/share/sounds/bt-deactivate.wav");
}