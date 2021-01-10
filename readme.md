# Dependencies
- SDL2
- SDL_ttf
- SDL_image
- libconfig
- libmpdclient
- sdbus-c++
- asound
- curlpp
- spdlog
- wiringPi

# TODO
1. Bluetooth
   - replace BT connect and disconnect sounds
   - move BT connect and disconnect sounds to the res folder
2. Alsa
   - Add equalizer to alsa configuration
   - http://www.gerrelt.nl/RaspberryPi/wordpress/equalizer/
   - https://www.hifiberry.com/docs/software/guide-adding-equalization-using-alsaeq/
3. Resilience
   - add WiFi auto reconnect
   - add option for manual shutdown
   - add option to display Error messages
4. Display
   - implement Bluelight Filter during night hours
5. Other
   - measures to speedup boot and reduce power consumption: https://github.com/megatron-uk/sdlRFController
   - make the sd card readonly; only useful with buildroot: https://learn.adafruit.com/read-only-raspberry-pi/
   - maybe use buildroot to setup a minimal image https://buildroot.org/

# Setup Steps
- Use: Raspberry Pi OS Lite
- Activate SSH by placing a empty ssh file in /boot before the first launch
- in /boot/config.txt add dtoverlays for the rotary encoder und gpio-keys https://github.com/raspberrypi/firmware/blob/master/boot/overlays/README
- in /boot/config.txt set spi=on
- in /boot add a wpa_supplicant.conf file with the basic Wifi configuration for a Headless setup https://www.raspberrypi.org/documentation/configuration/wireless/headless.md
- install and compile given dependencies
- enable passwordless auto login


## Systemd Configuration
- to setup systemd autolaunch on boot and auto restart service
- ```sudo cp ./res/systemd/_kitchensound.servcie /lib/systemd/system/kitchensound.service```
- ```sudo chmod 644 /lib/systemd/system/kitchensound.service```
- ```sudo systemctl daemon-reload```
- ```sudo systemctl enable kitchensound.service```
- reboot

## Alsa Configuration
https://alsa.opensrc.org/Softvol
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

## MPD Configuration
https://www.musicpd.org/doc/html/user.html#configuring-audio-outputs
https://www.musicpd.org/doc/html/plugins.html#output-plugins
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

## Bluetooth - Audio + Metadata Works
- BT Audio Setup Skript: https://github.com/nicokaiser/rpi-audio-receiver/blob/master/install-bluetooth.sh
- https://github.com/Arkq/bluez-alsa
- adjust ``bluealsa-aplay.service`` such that ``bluealsa-aplay``  is called with ``-d volumedev``
- Sounds:
  - BT On: https://www.zapsplat.com/wp-content/uploads/2015/sound-effects-41945/zapsplat_multimedia_alert_musical_warm_arp_006_46195.mp3?_=1
  - BT Off: https://www.zapsplat.com/wp-content/uploads/2015/sound-effects-41945/zapsplat_multimedia_alert_prompt_feedback_tone_simple_sine_chime_2_tone_007_45759.mp3?_=1

- Auf System DBus org.bluez / das ObjectManager interface auf änderungen monitoren; Speziell an neuen BT geräten interessiert sein
- Dann unter /org/bluez/hci0/dev_xxxxx mithilfe der org.freedesktop.DBus.Properties interface auf Property Änderungen warten; Speziell von interesse sind hier org.bluez.Device1.Connected sowie org.bluez.MediaControl1.Player
- Zuletzt bei verfügbarem player unter /org/bluez/hci0/dev_xxxxx/playerX mit dem gleichen Interface auf die property org.bluez.MediaPlayer1.Track watchen

## DBus
- Um per DBus auf die BT Infos zuzugreifen, kann man die Anwendung etweder als root launchen oder dem user "pi" erlauben die DBus destination "org.bluez" zu callen
- für Option zwei das File ``./res/dbus/_...`` nach ``/usr/share/dbus-1/system.d/`` kopieren
- dbus debugging with ``busctl``

## pHAT DAC - Works
- https://pinout.xyz/pinout/phat_dac# + Ground unten links
- https://www.hifiberry.com/docs/software/configuring-linux-3-18-x/

## Joy-IT Rotary Encoder - Works
- Mit DToverlay, geht auch analog für buttons
- https://blog.ploetzli.ch/2018/ky-040-rotary-encoder-linux-raspberry-pi/
- Dann kann man mit open(“/dev/input/event0”, O_READONLY | O_NONBLOCKING) nen file descriptor bekommen das read() wird damit non-blocking, falls was gelesen werden kann wird eine input_event structure gefüllt

## MX Cherry Red Keys - Works
- Mit DTOverlay kann jeder button wie eine normale Tastatur Taste angebunden werden
- Da die GPIO pins schon pull up /down wiederstände haben kann man die direkt anschließen
- Analog zum Rot Enc. kann man die als nonblocking readonly devices öffnen das value Attribute der input_event struct wird dabei 0 = Release, 1 = press, 2 = autorepeat

## Display - Works
- display driver and manual overlay + fbcp: https://github.com/notro/fbtft/wiki/FBTFT-RPI-overlays
- https://github.com/tasanakorn/rpi-fbcp
- https://github.com/philenotfound/rpi_elecfreaks_22_tft_dt_overlay
- Rendering mit SDL https://stackoverflow.com/questions/57672568/sdl2-on-raspberry-pi-without-x
- using pwm to dimm the display leads to display and audio distortions

## Internet Radio
- Stations Index API: https://de1.api.radio-browser.info/
- fetch additional metadata for songs https://www.discogs.com/developers