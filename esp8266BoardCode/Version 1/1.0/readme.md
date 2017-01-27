wiring instrucitions

if you want to use the esp8266 with pins, 4 5 and 14 for the RGB led, and pin 13 for the temp probe, youll need to setup your setup like this




****** STANDARD ******

PIN 4 - Red Leg --- setColor(255, 0, 0); // red
PIN 5 - Green Leg --- setColor(0, 255, 0); // Green
PIN 14 - Blue Leg --- setColor(0, 0, 255); // blue


****** MY WAY ******

PIN 5 - Red Leg --- setColor(0, 255, 0); // red
PIN 4 - Green Leg --- setColor(255, 0, 0); // Green
PIN 14 - Blue Leg --- setColor(0, 0, 255); // blue

Just throw the yellow wire from the ds18b20 onto pin 13 and that should work fine.

note the three LEDs need around 270ohm resistors

reference

https://cdn-learn.adafruit.com/downloads/pdf/adafruit-arduino-lesson-3-rgb-leds.pdf
