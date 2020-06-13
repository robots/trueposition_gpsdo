# TruePosition GPSDO monitor

Simple monitor device for trueposition gpsdo. 

Tested only with version 12.1.1 of firmware/bootloader.

## What does it do?

It boots up GPSDO by sending PROCEED command to bootloader. And it monitors status
of antenna, 10mhz and 1hz signal, and actual device state.

LCD display shows current GPS time and date converted to UTC, temperature, and state.

If abnormal state is detected symbol is displayed next to current date.
- ~ = 10MHz signal trouble
- p = 1Hz signal trouble
- Y = antenna trouble.

After pressing button, survey mode is entered. LCD displays survey time left.

## What do I need?
> http://www.ebay.de/itm/232249184432
> http://www.ebay.de/itm/232014092179
> button
> STM32 programmer or 3v ttl serial cable - to program the mcu.

### Pinout
PA8 - PPSin - pin 11 on gps
PA9 - UARTtx - pin 3 on gps
PA10 - UARTrx - pin 7 on gps

PB6 - SCL - to display
PB7 - SDA - to display

PC13 - led on board
