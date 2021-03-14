# TODO
0. Kitchensound
   - add option to display Error messages / last log output
   - refactor threading away by handling everything non-blocking sequentially
    - mpd controller is done
    - bt_controller
    - cache_manager
   - add a new time helper class to get rid of all the manual timers 
    - cache/resource manager
   
   - remove ctime dependency in favor of std::chrono
1. Alsa
    - Add equalizer to alsa configuration
    - http://www.gerrelt.nl/RaspberryPi/wordpress/equalizer/
    - https://www.hifiberry.com/docs/software/guide-adding-equalization-using-alsaeq/
2. Resilience
    - add WiFi auto reconnect
5. Other
    - maybe use buildroot to setup a minimal image https://buildroot.org/

## Internet Radio
- Stations Index API: https://de1.api.radio-browser.info/
- fetch additional metadata for songs https://www.discogs.com/developers