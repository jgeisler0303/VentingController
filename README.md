# Application

This is the firmware for my for my basement humidity sensitive venting control system. It runs on the Arduino Duemilanove w/ATmega328 compatible iBoard from ITeadStudio (http://imall.iteadstudio.com/im120410001.html). With the boards integrated Ethernet connection all data is logged to https://xively.com/feeds/56748.
Venting is activated based on inside and outside temperature and humidity measurements. To make sure the venting will effectively lower the humidity in my basement a smart algorithm determines whether the absolute humidity (http://en.wikipedia.org/wiki/Humidity) outside is lower than inside. With an adaptive threshold it will also try to find the best time of the day to achieve maximum humidity reduction with a minimum of venting cycles. The algorithm also tries to not increase or decrease the inside temperature too much above resp. below a seasonally variable target temperature. And finally, the venting is limited to 15 cycles in 24h. One venting cycle comprises 10min. of active venting (fan and/or window opener) followed by a 20min. settling period.

# Hardware

Temperature and humidity are measured using ubiquitous DHT22 sensors. An MCP23S17 16bit IO-port expander is used to control the solid-state relays for the fans and windows openers and to read in an additional rain sensor and a pulse from my gas meter. Info is displayed on a 20x4 character LCD with a simple scrolling scheme using one input button. Currently there is no documentation on how to connect these to the iBoard.

# Software

The code is very messy, undocumented and quite special to my needs. So, beware, no satisfaction guaranty given! If you are interested or have any questions don't hesitate to open an Issue.

All configuration is hard-coded and any change requires a firmware flashing.

## Compiling and Running
The code is an Arduino sketch with several modules (tabs). Open the main tab (VentingController.ino) in the Arduino IDE and the other files will be opened automatically.

In order to successfully compile the code you need to put several extra libraries into Arduino sketchbook/libraries folder:
* DHT22 please download special version from: https://github.com/jgeisler0303/Arduino-DHT22
* TimeX.h: http://playground.arduino.cc/Code/Time
* TimedAction: http://playground.arduino.cc/Code/TimedAction oder now https://github.com/Glumgad/TimedAction
* LiquidCrystal_I2C please download special version from: https://github.com/jgeisler0303/Arduino-LiquidCrystal-I2C-library
* StopWatch: http://playground.arduino.cc/Code/StopWatchClass
* Mcp23s17 please download special version from: https://github.com/jgeisler0303/Mcp23s17


Finally you need to fill in your Xively feed authentication data and local ip address into Example_XivelyKey.h and rename it to XivelyKey.h

## Memory efficient Xively Communication
One of the modules is a memory efficient version of ERxPachube library (https://code.google.com/p/pachubelibrary/) for Pachube/Cosm/Xively GET and PUT API communication. Though, this module cannot be used as a library in its current state I may convert it into a librar in the future.

The memory efficiency is achieved by not preparing the whole message in RAM before sending it. The problem is that the size of the message body must be known before it is sent. To avoid having to prepare the whole message to know its size, a fixed pre estimated size is sent instead. Then the message is generated and directly sending character by character while they are created, thus not taking up any memory. Finally, to keep the anounced message size, the message is padded with blanks until the size is reached.

This has two drawbacks: first, a fixed size must be chosen that will not be exeeded by the message under all circumstances. Secondly, Time is wasted by having to send the padding blanks. But the memory saving is considrable and the upper bound of the message size can be estimated relatively easy. It is configured via the COSM_CONTENT_LENGHT define in config.h. Its current value is 256 characters max.

With this method the maximum number of streams could be increased from 5 to 32. 

Also, this pseudo library adds the possibility to extract the current time from the HTTP header.
