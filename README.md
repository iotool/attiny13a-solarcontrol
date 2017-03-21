# ATtiny13A Solar Controller with Arduino IDE

* ADC3 = read vcc (using a 1.6V led as Vref)
* ADC4 = info led (blinkcode: 1x 2.7V, 2x 3.0V, 3x 3.6V, 4x 4.2V)
* PWM0 = switch 1 (on over 3.3V / off below 2.6V)
* PWM1 = switch 2 (on over 3.0V / off below 3.0V)

This ATtiny solar controller charges a supercap to 3.6 voltage and power on a ESP8266. 
After reaching 3.6V the ESP866 can power on by a MOSFET to upload sensor values.
If the supercap goes under 2.6V the ESP8266 will power off and charging restarts. 

## upload binrary via Arduino as ISP

  C:\arduino-1.6.8\hardware\tools\avr/bin/avrdude -CC:\arduino-1.6.8\hardware\tools\avr/etc/avrdude.conf -v -pattiny13 -cstk500v1 -PCOM4 -b19200 -Uflash:w:C:\Temp\attiny13a_solar-control.ino.hex:i

## unlimited tenor

This project using a supercap as a substitute for rechargeable battery charged by a solar panel. 
You will need 15 minutes to charge a 0.47F supercap to 3.6V with a 6V solar panel to consume 85mAs.
A supercap 1.5F 5.5V needs 45 minutes and stores 272mAs energy while drop down voltage from 3.6V to 2.6V.
With 272mAs you can run a ESP8266 for round about 6 seconds to messure and upload sensor values.
