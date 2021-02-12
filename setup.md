# Setup

### 1. OS Setup
1. Download and flash Raspberry Pi OS Lite
2. Activate SSH by placing a empty ssh file in /boot before the first launch
3. in /boot/config.txt add dtoverlays for the rotary encoder and gpio-keys for the mx-red switches https://github.com/raspberrypi/firmware/blob/master/boot/overlays/README
4. in /boot/config.txt set spi=on to allow the display to work
5. in /boot add a wpa_supplicant.conf file with the basic Wifi configuration for a headless setup https://www.raspberrypi.org/documentation/configuration/wireless/headless.md
6. after the first boot enable passwordless auto login in ``raspi-config``
7. configure the appropriate time zone and keyboard language in ``raspi-config``
8. enable the ``testing`` repository in ``/etc/apt/sources.list``
9. update the repositories using ``sudo apt update``

### 2. Install Dependencies
- SDL2 >= 2.0.12 (self compiling is recommended, as the debian package also installs the x-server which is unnecessary)
- SDL_ttf >= 2.0.15 (self compiling is needed when self compiling SDL2)
- SDL_image >= 2.0.5 (self compiling is needed when self compiling SDL2)
- sdbus-c++ >= 0.8.3 (self compile needed; not in the repos)
- libconfig (i self compiled 1.7.2 but 1.5 from the repos, stable should do as well)
- libmpdclient (from repo, stable, currently 2.16)
- libasound (from repo, stable, currently 1.1.8)
- libao (from repo, stable, currently 1.2.2)
- libmpg123 (from repo, stable, currently 1.25)
- libcurlpp (from repo, stable, currentl 0.8.1)
- libspdlog (from repo, stable, currently 1.3.1)
- libgpiod (from repo, stable, currently 1.2)

1. to install recomended things from the repo use ``sudo apt install libconfig-dev libmpdclient-dev libasound-dev libao-dev libmpg123-dev libcurlpp-dev libspdlog-dev libgpiod-dev``
2. install the development base packages to compile SDL, it's plugins and sdbus-c++
3. download the appropriate source files, compile and install the libs


## 3. Systemd Configuration
- to setup systemd autolaunch on boot and auto restart service use the following commands to copy the provided systemd service configuration
- ``sudo cp ./res/systemd/_kitchensound.servcie /lib/systemd/system/kitchensound.service``
- ``sudo chmod 644 /lib/systemd/system/kitchensound.service``
- ``sudo systemctl daemon-reload``
- ``sudo systemctl enable kitchensound.service``
- ``sudo reboot``

## 4. Alsa Configuration
This explanatory example demonstrates the alsa configuration setup with an additional softvol device
to allow for volume control (also see the [doc](https://alsa.opensrc.org/Softvol))
````
pcm.!default {
    type hw card 0
}
ctl.!default {
    type hw card 0
}
pcm.volumedev {
    type softvol
    slave.pcm "default"
    control.name "Master"
    control.card 0
}
````

## 5. MPD Configuration
Explanatory example of minimal mpd configuration specifically to only load the necessary plugins for playing network streams.
Also refer to the official doc [here](https://www.musicpd.org/doc/html/user.html#configuring-audio-outputs) and 
[here](https://www.musicpd.org/doc/html/plugins.html#output-plugins).
```
bind_to_address     "localhost"
intput {
    plugin "curl"
}
input {
    enabled "no"
    plugin  "qobuz"
}
input {
    enabled "no"
    plugin  "tidal"
}
decoder {
    plugin  "hybrid_dsd"
    enabled "no"
}
audio_output {
    type    "alsa"
    name    "kitchensound output"
    device  "volumedev"
    auto_resample   "no"
    mixer_control   "Master"
}
```

## 6. Bluetooth - Audio + Metadata
- BT Audio Setup Skript: https://github.com/nicokaiser/rpi-audio-receiver/blob/master/install-bluetooth.sh
- https://github.com/Arkq/bluez-alsa
- adjust ``bluealsa-aplay.service`` such that ``bluealsa-aplay``  is called with ``-d volumedev``
- Sounds: run the ``res/sound/sound_fetcher.sh`` script to download the bluetooth sounds I use from [Zapsplat](https://zapsplat.com)

## 7. DBus
- Um per DBus auf die BT Infos zuzugreifen, kann man die Anwendung etweder als root launchen oder dem user "pi" erlauben die DBus destination "org.bluez" zu callen
- für Option zwei das File ``./res/dbus/_...`` nach ``/usr/share/dbus-1/system.d/`` kopieren
- dbus introspection/analysis is best done with ``busctl``

## 8. PolKit
- to enable system reboot and shutdown without manual ssh login and running the software as root (using sudo) a local exception is needed
- therefore use the following excerpt (also found in ``res/polkit``:
```
[Allow the user Pi to shutdown]
Identity=unix-user:pi
Action=org.freedesktop.login1.reboot;org.freedesktop.login1.reboot-multiple-sessions;org.freedesktop.login1.power-off;org.freedesktop.login1.power-off-multiple-sessions;
ResultAny=yes
```
- the excerpt is a combination of [StackOverflow Solutions](https://askubuntu.com/questions/493627/power-button-shutdown-permission-override) and official documentation [Doc](https://www.freedesktop.org/software/polkit/docs/0.105/pklocalauthority.8.html)

## 9. GPIO
- to enable gpio access to the software without launching it as root add the user to the ``gpio`` group by calling ``sudo usermod -a -G gpio <myuser>``

## 10. pHAT DAC
- https://pinout.xyz/pinout/phat_dac# + I use the ground pin on the lower left
- https://www.hifiberry.com/docs/software/configuring-linux-3-18-x/

## 11. Joy-IT Rotary Encoder
- Mit DToverlay, geht auch analog für buttons
- https://blog.ploetzli.ch/2018/ky-040-rotary-encoder-linux-raspberry-pi/
- Dann kann man mit open(“/dev/input/event0”, O_READONLY | O_NONBLOCKING) nen file descriptor bekommen das read() wird damit non-blocking, falls was gelesen werden kann wird eine input_event structure gefüllt

## 12. MX Cherry Red Keys
- Mit DTOverlay kann jeder button wie eine normale Tastatur Taste angebunden werden
- Da die GPIO pins schon pull up /down wiederstände haben kann man die direkt anschließen
- Analog zum Rot Enc. kann man die als nonblocking readonly devices öffnen das value Attribute der input_event struct wird dabei 0 = Release, 1 = press, 2 = autorepeat

## 13. Display
- display driver and manual overlay + fbcp: https://github.com/notro/fbtft/wiki/FBTFT-RPI-overlays
- https://github.com/tasanakorn/rpi-fbcp
- https://github.com/philenotfound/rpi_elecfreaks_22_tft_dt_overlay
- [Rendering with SDL](https://stackoverflow.com/questions/57672568/sdl2-on-raspberry-pi-without-x)
- experiments to dimm the display using pwm have failed or led to audio distortions