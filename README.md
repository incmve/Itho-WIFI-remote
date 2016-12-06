# IthoEcoFanRFT
Cloned from https://github.com/supersjimmie/IthoEcoFanRFT

I used A CC1101 module, might work with others like CC1150.
```
Connections between the CC1101 and the ESP8266:
CC11xx pins    ESP pins   		Description
*  1 - VCC        VCC         	3v3
*  2 - GND        GND     		Ground
*  3 - MOSI       13=D7  		Data input to CC11xx
*  4 - SCK        14=D5			Clock pin
*  5 - MISO/GDO1  12=D6			Data output from CC11xx / serial clock from CC11xx
*  6 - GDO2       ? 			Programmable output
*  7 - GDO0       ?  			Programmable output
*  8 - CSN        15=D8 		Chip select / (SPI_SS)
```


Beware that the CC11xx modules are 3.3V (3.6V max) on all pins!
This won't be a problem with an ESP8266, but for Arduino you either need to use a 3.3V Arduino or use levelshifters and a separate 3.3V power source.

For SPI, pins 12-15 ()aka D5-D8) are used, a larger ESP8266 model like (but not only) the ESP-03, ESP-07, ESP-12(D, E) or a NodeMCU board is required.

Added a web interface and simple API to control it from something like https://pimatic.org/

![alt tag](https://github.com/incmve/Itho-WIFI-remote/blob/master/Images/cntrl panel.JPG)

Also able to give commands by USB

![alt tag](https://github.com/incmve/Itho-WIFI-remote/blob/master/Images/serial.JPG)


API work like this:
```
http://192.168.x.x/api?action=Low
http://192.168.x.x/api?action=Medium
http://192.168.x.x/api?action=High
http://192.168.x.x/api?action=Timer
http://192.168.x.x/api?action=Learn
http://192.168.x.x/api?action=reset&value=true
```

Use at your own RISK!!