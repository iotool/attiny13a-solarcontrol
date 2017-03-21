# ATtiny13A Solar-Control

* ADC3 = read vcc (using a 1.6V led as Vref)
* ADC4 = info led (blinkcode: 1x 2.7V, 2x 3.0V, 3x 3.6V, 4x 4.2V)
* PWM0 = switch 1 (on over 3.3V / off below 2.6V)
* PWM1 = switch 2 (on over 3.0V / off below 3.0V)

This ATtiny solar controller charges a supercap and power on / off a ESP8266. 
After reaching 3.6V the ESP866 can power on by a MOSFET and upload sensor values.
If the supercap goes under 2.6V the ESP8266 will power off and the charges restart. 

## Upload binrary via Arduino as ISP

  C:\arduino-1.6.8\hardware\tools\avr/bin/avrdude -CC:\arduino-1.6.8\hardware\tools\avr/etc/avrdude.conf -v -pattiny13 -cstk500v1 -PCOM4 -b19200 -Uflash:w:C:\Temp\attiny13a_solar-control.ino.hex:i
