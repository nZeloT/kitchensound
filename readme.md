# Kitchensound

A small software driving my kitchenradio. It uses:
- Raspberry Pi Zero W
- Two Cherry MX Red
- One Joy-IT Rotary Encoder
- Pimorino pHat
- TPA3116D2 driven amp
- 2.2" ili9341 driven display
- Relais to turn the amp on or off
- 12v to 5v step down converter
- 12v 4a power supply

The software is written in C++ and currently features:
- Displaying a 24 hour time when inactive
- Playing a given radio stream (using MPD)
- Reading and displaying metadata from the played radio stream
- Playing audio from a connected Bluetooth device (using the Pi0 W built in Bluetooth adapter and bluealsa)
- Reading and displaying metadata from the played audio stream (using DBus or MPD)
- Displaying statistics about system and program uptime
- Restating or shutting down without using SSH
- Can be run without sudo with a few tweaks
- Optional display off / standby time during configurable night hours
- Uses a event driven (epoll) single thread mechanism to try and minimize system utilization

Setup and configuration instructions can be found in [setup.md](./setup.md)