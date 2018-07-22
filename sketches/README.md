Welcome to the 503 Party
------------------------
So, you have a 503 banglet, now what?  You may notice a striking similarity to the Adafruit Feather.  This is because the banglet board pretty much a Feather modified with some extra banglety goodies.  These instructions will take you through flashing your own custom arduino sketch to your banglet.


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

