# Setup

## 1. OS Setup
### 1.1. Before first system boot
1. Download and flash Raspberry Pi OS Lite to microSD Card
2. Activate SSH by placing a empty file named ``ssh`` in ``/boot``
3. Add a ``wpa_supplicant.conf`` file in ``/boot`` with the [basic Wifi configuration](https://www.raspberrypi.org/documentation/configuration/wireless/headless.md) for a headless setup

### 1.2. After the first system boot
1. Enable passwordless auto login in ``raspi-config``
2. Configure the appropriate time zone and keyboard language in ``raspi-config``
3. Enable the ``testing`` repository in ``/etc/apt/sources.list``
4. Update the repositories using ``sudo apt update``
5. Upgrade upgradeable packages to the new version using ``sudo apt upgrade``
6. Deactivate the swap file using ``sudo swapoff -a``

## 2. Dependencies
### 2.1. List of Required Dependencies
- MPD >= 0.22 (self compiling is recommended)
- SDL2 >= 2.0.12 (self compiling is recommended, as the debian package also installs the x-server which is unnecessary)
- SDL_ttf >= 2.0.15 (self compiling is needed when self compiling SDL2)
- SDL_image >= 2.0.5 (self compiling is needed when self compiling SDL2)
- libsystemd (from repo, stable, currently 241)
- libconfig (i self compiled 1.7.2 but 1.5 from the repos, stable should do as well)
- libmpdclient (from repo, stable, currently 2.16)
- libasound (from repo, stable, currently 1.1.8)
- libao (from repo, stable, currently 1.2.2)
- libmpg123 (from repo, stable, currently 1.25)
- libcurl (from repo, stable, currently 7.64)
- libspdlog (from repo, stable, currently 1.3.1)
- libgpiod (from repo, stable, currently 1.2)

### 2.2. Installing required dependencies from the repos
1. to install recommended things from the repo use ``sudo apt install libconfig-dev libmpdclient-dev libasound-dev 
   libao-dev libmpg123-dev libcurl4-gnutls-dev libspdlog-dev libgpiod-dev libsystemd-dev``

### 2.3. Compiling further required dependencies
#### 2.3.1. MPD
1. Download and extract MPD source tarball
2. Download and extract libboost source tarball or install a version ```> 1.68``` from the repos
3. From the repos install ``meson ninja-build libflac-dev libvorbis-dev libopus-dev libogg-dev libaudiofile-dev libsndfile1-dev libfaad-dev libsamplerate0-dev libcurl4-gnutls-dev libasound2-dev libsystemd-dev libicu-dev``
4. Define `BOOST_ROOT` and `BOOST_INCLUDEDIR` using the ``export`` command accordingly
5. Manipulate the `src/lib/alsa/meson.build`, `src/lib/curl/meson.build` to include ``boost_dep`` as static library dependency
6. Invoke meson using ``meson . build/release --buildtype=release``
7. Configure the setup with ``meson configure build/release/ -Dzzip=disabled -Dzlib=disabled -Dzeroconf=disabled -Dyajl=disabled -Dwildmidi=disabled -Dwebdav=disabled -Dwavpack=disabled -Dwave_encoder=false -Dvorbisenc=disabled -Dupnp=disabled -Dudisks=disabled -Dtwolame=disabled -Dtrmor=disabled -Dtidal=disabled -Dsqlite=disabled -Dsoxr=disabled -Dsoundcloud=disabled -Dsolaris_output=disabled -Dsndio=disabled -Dsidplay=disabled -Dshout=disabled -Dshine=disabled -Drecorder=false -Dqobuz=disabled -Dpulse=disabled -Dpipe=false -Doss=disabled -Dopenal=disabled -Dnfs=disabled -Dneighbor=false -Dmpcdec=disabled -Dmodplug=disabled -Dmms=disabled -Dmikmod=disabled -Dlibmpdclient=disabled -Dlame=disabled -Djack=disabled -Diso9660=disabled -Dinotify=false -Dhttpd=false -Dhtml_manual=false -Dgme=disabled -Dfluidsynth=disabled -Dfifo=false -Ddsd=false -Ddocumentation=disabled -Dexpat=disabled -Ddatabase=false -Dcdio_paranoia=disabled -Dbzip2=disabled -Dao=disabled -Dadplug=disabled -Dcue=false -Dchromaprint=disabled``
8. Start compiling with ``ninja -C build/release``
9. Test the mpd build by starting it and using ``mpc`` (from the repos) to control it
10. Install using ``ninja -C build/release install``
11. Remove the now unnecessary header files for the previously installed libraries (essentially remove all installed ``*-dev`` packages but keep the libraries themself)
12. Delete the extracted boost files
13. Delete the extracted MPD sources

#### 2.3.2. SDL2
1. Download the current SDL2 tarball and extract it
2. Configure with ``--disable-atomic --disable-audio --disable-joystick --disable-haptic --disable-sensor --disable-power --disable-threads --disable-timers --disable-oss --disable-alsa --disable-alsa-shared --disable-jack --disable-jack-shared --disable-esd --disable-esd-shared --disable-pulseaudio --disable-pulseaudio-shared --disable-arts --disable-arts-shared --disable-nas --disable-nas-shared --disable-sndio --disable-sndio-shared --disable-fusionsound --disable-fusionsound-shared --disable-diskaudio --disable-dummyaudio --disable-libsamplerate --disable-libsamplerate-shared --disable-video-vulkan --disable-video-rpi --disable-video-opengles1 --disable-video-wayland --disable-wayland-shared --disable-video-x11 --disable-x11-shared --disable-video-x11-xcursor --disable-video-x11-xdbe --disable-video-x11-xinerama --disable-video-x11-xinput --disable-video-x11-xrandr --disable-video-x11-scrnsaver --disable-video-x11-xshape --disable-video-x11-vm --disable-video-vivante --disable-video-cocoa --disable-video-metal --disable-video-directfb --disable-directfb-shared --disable-video-dummy --disable-dbus --disable-ime --disable-ibus --disable-fcitx --disable-joystick-mfi --disable-pthreads --disable-pthread-sem --disable-wasapi --disable-sdl-dlopen --disable-hidapi --disable-joystick-virtual``
3. Build and install sdl using ``make``
4. Remove the downloaded tarball and extracted files

#### 2.3.3. SDL_image, SDL_ttf
1. no special instructions, follow their guide
2. delete tarball and extracted files after installing

## 3. Setup cross compilation for faster kitchensound compile speed (only needed when actively developing)
1. Follow the instructions [here](https://github.com/Pro/raspi-toolchain/)
2. Use the provided cross build script

## 4. Hardware Configuration
### 4.1. pHAT DAC
- I'm using the ``Pimorino pHat DAC``. You may need to adjust this to the DAC you are using
- I wired according to the [pinout documentation](https://pinout.xyz/pinout/phat_dac) and used the ground pin in the lower left (Physical Pin 39)
- Finally add the line ``dtoverlay=hifiberry-dec`` to the ``/boot/config.txt``
- Sett [HifiBerry](https://www.hifiberry.com/docs/software/configuring-linux-3-18-x/) for info about setting up other DAC'S.

### 4.2. Joy-IT Rotary Encoder
- Adapt and add the following line to ``/boot/config.txt`` ``dtoverlay=rotary-encoder,pin_a=<gpio_pin>,pin_b=<gpio_pin>,relative_axis=1``
- Have a [read](https://blog.ploetzli.ch/2018/ky-040-rotary-encoder-linux-raspberry-pi/) for more details
- Also see the dtoverlay [src-file](https://github.com/raspberrypi/linux/blob/c9226080e513181ffb3909a905e9c23b8a6e8f62/arch/arm/boot/dts/overlays/rotary-encoder-overlay.dts) in the Raspberry Pi source repositories ([doc](https://github.com/raspberrypi/linux/blob/c9226080e513181ffb3909a905e9c23b8a6e8f62/Documentation/devicetree/bindings/input/rotary-encoder.txt))
- Check the ``/dev/eventX`` files to see which one is the relative axis and adapt the [config file](./config.conf) accordingly.

### 4.3. MX Cherry Red Keys and Rotatry Encoder Center Button
- Adapt and add the following line to ``/boot/config.txt`` ``dtoverlay=gpio-key,gpio=<gpio_pin>,label="My nice key",keycode=<linux keycode>``
- Compare the [src-file](https://github.com/raspberrypi/linux/blob/c9226080e513181ffb3909a905e9c23b8a6e8f62/arch/arm/boot/dts/overlays/gpio-key-overlay.dts) and the [official documenation](https://github.com/raspberrypi/linux/blob/16991af20bd821ecba1eae2fe11ddbcafa18f164/arch/arm/boot/dts/overlays/README) for details.
- See [this](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h) reference for possible ``keycode`` values.
- Repeat this for the two Cherry BUttons and the Center Button of the rotatry encoder

### 4.4. Display
- I'm using a 2.2" SPI TFT (320x240 pixels); Adapt to your needs
- Within ``/boot/config.txt`` add the line ``spi=on``
- display driver and manual overlay + fbcp: https://github.com/notro/fbtft/wiki/FBTFT-RPI-overlays
- https://github.com/tasanakorn/rpi-fbcp
- https://github.com/philenotfound/rpi_elecfreaks_22_tft_dt_overlay
- [Rendering with SDL](https://stackoverflow.com/questions/57672568/sdl2-on-raspberry-pi-without-x)
- Experiments to dimm the display using pwm have failed or led to audio distortions


## 5. Software Configuration

### 5.1. Systemd
- to setup systemd autolaunch on boot and auto restart service use the following commands to copy the provided systemd service configuration
- ``sudo cp ./res/systemd/_kitchensound.servcie /lib/systemd/system/kitchensound.service``
- ``sudo chmod 644 /lib/systemd/system/kitchensound.service``
- ``sudo systemctl daemon-reload``
- ``sudo systemctl enable kitchensound.service``
- ``sudo reboot``

### 5.2. Alsa
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

### 5.3. MPD
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

### 5.4. Bluetooth - Audio
- BT Audio Setup Skript: https://github.com/nicokaiser/rpi-audio-receiver/blob/master/install-bluetooth.sh
- https://github.com/Arkq/bluez-alsa
- adjust ``bluealsa-aplay.service`` such that ``bluealsa-aplay``  is called with ``-d volumedev``
- Sounds: run the ``res/sound/sound_fetcher.sh`` script to download the bluetooth sounds I use from [Zapsplat](https://zapsplat.com)

### 5.5. DBus
- Um per DBus auf die BT Infos zuzugreifen, kann man die Anwendung etweder als root launchen oder dem user "pi" erlauben die DBus destination "org.bluez" zu callen
- für Option zwei das File ``./res/dbus/_...`` nach ``/usr/share/dbus-1/system.d/`` kopieren
- dbus introspection/analysis is best done with ``busctl``

### 5.6. PolKit
- to enable system reboot and shutdown without manual ssh login and running the software as root (using sudo) a local exception is needed
- therefore use the following excerpt (also found in ``res/polkit``:
```
[Allow the user Pi to shutdown]
Identity=unix-user:pi
Action=org.freedesktop.login1.reboot;org.freedesktop.login1.reboot-multiple-sessions;org.freedesktop.login1.power-off;org.freedesktop.login1.power-off-multiple-sessions;
ResultAny=yes
```
- the excerpt is a combination of [StackOverflow Solutions](https://askubuntu.com/questions/493627/power-button-shutdown-permission-override) and official documentation [Doc](https://www.freedesktop.org/software/polkit/docs/0.105/pklocalauthority.8.html)

### 5.7. GPIO
- to enable gpio access to the software without launching it as root add the user to the ``gpio`` group by calling ``sudo usermod -a -G gpio <myuser>``

## 6. Kompiling Kitchensound
