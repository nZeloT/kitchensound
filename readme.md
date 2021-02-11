# Kitchensound

A small software driving my kitchenradio. It uses:
- Raspberry Pi Zero W
- Two Cherry MX Red
- One Joy-IT Rotary Encoder
- Pimorino pHat
- TPA3116D2 driven amp
- 2.2" ili9341 driven display
- Relais to turn the amp on or off

The software is written in pure C++ and currently features:
- Displaying a 24 hour time when inactive
- Playing a given radio stream (using MPD)
- Reading and displaying metadata from the played radio stream
- Playing audio from a connected Bluetooth device (using the Pi0 W built in Bluetooth adapter and bluealsa)
- Reading and displaying metadata from the played audio stream (using DBus)
- Displaying statistics about system and program uptime
- Can be run without sudo with a few tweaks
- Optional display off / standby time during configurable night hours

Setup and configuration instructions can be found in ``setup.md``