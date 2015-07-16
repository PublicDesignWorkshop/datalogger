# Datalogger

This repository has three branches representing three different kinds of power-saving dataloggers that record analog input to SD cards, and timestamps from a Real-Time-Clock module. All of these are based on the Datalogger sketch included as an example with the Arduino software.

The Real-Time Clock used in these files is here: [SparkFun DeadOn RTC Breakout - DS3234](https://www.sparkfun.com/products/10160)

The libraries that these sketches use are the SD card library included with the Arduino IDE, [ManiacBug's fork of RTClib](https://github.com/maniacbug/RTClib) and [Rodan's DS3234 Library](https://github.com/rodan/ds3234). For energy management in the *interrupt* branch, I used [Enerlib](http://playground.arduino.cc/Code/Enerlib).

###Minimizing current use
To reduce current draw, it'a best to avoid Arduino boards. They contain components (LEDs, voltage regulators, etc) that will draw power even in idle or sleep modes. Building an [Arduino on a breadboard](https://www.arduino.cc/en/main/standalone) will make it lower-power. I also avoid using external oscillators and flash the ATMEGA microcontroller with Arduino Lilypad firmware to use the 8MHz onboard clock.

![ATMEGA328P on a breadboard with SD card reader and DS3234](http://thomasjenkins.net/dropbox/arduino-breadboard.jpg)

To upload software, I use an FTDI cable. The voltage regulator is to drop the voltage supplied from the [5V FTDI cable](https://www.sparkfun.com/products/9718) to 3.3V, but it would be simpler to just use a [3.3V FTDI cable](https://www.sparkfun.com/products/9717).

For more on low-power Arduino in general, this is a helpful: [Adafruit: Low-Power WiFi Datalogging](https://learn.adafruit.com/low-power-wifi-datalogging).

##Master branch
The *master* branch is a basic datalogger that uses the DS3234 timer to store analog sensing data with a timestamp. 


##Sleep branch
The *sleep* branch uses the Arduino's internal watchdog timer to shut it down and re-enable it at various multiples of 8 seconds (the longest internal sleep duration). Because the microcontroller awakens so frequently, power savings are not great in this mode.

Here, sleeping is handled manually.

##Interrupt branch
the *interrupt* branch uses the alarm capability of the DS3234 to schedule when it should awaken from sleep. It sends a signal from SQW to an interrupt pin while the microcontroller is completely asleep. The MCU wakes up, logs data, and goes back to sleep.

Here, the enerlib library handles sleep.