# MageFlame

This is the code used on my Mage's Staff ( https://youtu.be/Gb1wH2Rldzw )

It expects 7 neopixel-compatible LEDS with their data pin connected to pin 6 of the Arduino.
It expects a momentary push-button or impact sensor to be connected to pin 3 of the Arduino.
It expects Analog Pin 0 to be empty, as it uses the float value for the random seed.

On startup it picks a random colour and simulates a flame on each neopixel.
Double-tapping the button/sensor will cause it to flash all LEDs white, pick a new random flame colour, and fade into the new flame colour.

Uses a stock HSV2RGB converter.
