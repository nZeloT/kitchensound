# Dependencies
- SDL2
- SDL_ttf
- SDL_image
- libconfig
- libmpdclient
- sdbus-c++
- asound
- curlcpp
- spdlog
- wiringPi

# TODO
1. Rendering
   - marquee texts which are to wide -- adjust browsing stations
   - load radio station images
2. Bluetooth
   - BT anzeige Name als Property Alias im Devie Interface lesen und reporten statt device ID
   - BT Connect + Disconnect sounds ersetzen; werden automatisch vom Setupskript installiert
   - Sounds in den res ordner moven
3. Alsa
   - Equalizer einbauen
   - http://www.gerrelt.nl/RaspberryPi/wordpress/equalizer/
   - https://www.hifiberry.com/docs/software/guide-adding-equalization-using-alsaeq/
4. Resilience
   - WiFi auto reconnect
   - Manual shutdown
   - Error message display
5. Display
   - Dimm
   - Bluelight Filter during night hours

# Setup Steps
- Use: Raspberry Pi OS Lite
- Activate SSH by placing a empty ssh file in /boot before the first launch
- in /boot/config.txt add dtoverlays for the rotary encoder und gpio-keys https://github.com/raspberrypi/firmware/blob/master/boot/overlays/README
- in /boot/config.txt set spi=on
- in /boot add a wpa_supplicant.conf file with the basic Wifi configuration for a Headless setup https://www.raspberrypi.org/documentation/configuration/wireless/headless.md
- compile libconfig, libmpdclient, SDL, SDL_ttf, SDL_image, sdbus-c++ in the given versions
- install libcurl-dev, mpd, wiringpi

## Systemd Configuration
- to setup systemd autolaunch service
- ```sudo cp ./res/systemd/_kitchensound.servcie /lib/systemd/system/kitchensound.service```
- ```sudo chmod 644 /lib/systemd/system/kitchensound.service```
- ```sudo systemctl daemon-reload```
- ```sudo systemctl enable kitchensound.service```
- reboot

## Alsa Konfiguration
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

## MPD Konfiguration
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
- Displaytreiber manuelles dtoverlay + fbcp: https://github.com/notro/fbtft/wiki/FBTFT-RPI-overlays
- https://github.com/tasanakorn/rpi-fbcp
- https://github.com/philenotfound/rpi_elecfreaks_22_tft_dt_overlay
- Rendering mit SDL https://stackoverflow.com/questions/57672568/sdl2-on-raspberry-pi-without-x
- Mehrere Versuche Backlight dimmbar zu machen mit PWM1 sind gescheitert; Bester versuch war mig pigpio; Gab aber ordentlich tearing und Farbverschiebungen; geht vermutlich erst wenn fbtft im Kernel das kann oder so

## Internet Radio
- Stations Index API: https://de1.api.radio-browser.info/
- Wiedergabe durch MPD mit pipe in snapcast
- Weitere Meta Daten können durch https://www.discogs.com/developers geladen werden; u.a. auch Cover Bilder

## DLNA/UPnP
- Renderer: https://github.com/hzeller/gmrender-resurrect
- Audio kann man direkt in snapcast pipen 
- evtl. ist auch ein Controller notwendig um die entsprechenden Metadaten anzuzeigen -> https://github.com/hzeller/upnp-display als beispiel

## Spotify
- Librespot: https://github.com/librespot-org/librespot
- Kann man direkt headless starten und in Snapcast pipen

## Other
- https://github.com/Spotifyd/spotifyd
- https://learn.adafruit.com/read-only-raspberry-pi/
- Energiespar und boot beschleunigung: https://github.com/megatron-uk/sdlRFController
- https://buildroot.org/