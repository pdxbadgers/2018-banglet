Welcome to the 503 Party
------------------------
So, you have a 503 banglet, now what?  You may notice a striking similarity to the Adafruit Feather.  This is because the banglet board is pretty much a Feather modified to fit the banglet form factor. What that means is that A0 is the only data pin you have access to and it goes straight to a neopixel strip. But you still have the whole suite of BT functionality that the Nordic NRF52 SoC has to offer. So you'll have to get creative with encoding your data into blinky lights ;).

Adafruit has a good collection of examples to get you started with writing your custom sketch. You can clone their repo here: https://github.com/adafruit/Adafruit_NRF52_Arduino

These instructions will take you through flashing your own custom arduino sketch to your banglet.

Installing the Feather board definitions
----------------------------------------

Paraphrased from here:

https://learn.adafruit.com/adafruit-feather-32u4-basic-proto/arduino-ide-setup

Download Arduino for your platform:

https://www.arduino.cc/en/Main/Software

In preferences add "Additonal Boards Manager URLs":

https://adafruit.github.io/arduino-board-index/package_adafruit_index.json


Tools > Board > Board Manager

Search for "Adafruit nrf52 by Adafruit"

Install the latest.


Restart Arduino IDE

Toors > Board > Adafruit Feather 32u4

Installing the Neo Pixel library
--------------------------------
To light up the neopixels, you need to install the Neo Pixel libraries.  Luckily you should be able to find it pretty quick in the library manager.

Sketch > Include Libraries > Manage Libraries

Adafruit Neo Pixel

