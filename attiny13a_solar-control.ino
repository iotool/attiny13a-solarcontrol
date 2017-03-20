// ATtiny13a SolarControl
// 
// ADC3 = VCC der Solarzelle ueber 1.6V LED als Vref messen
// ADC4 = optionale LED mit Blinkcode fuer Spannung (2.7/3.0/3.7/4.2V)
// PWM0 = Schalter ein ~ VCC > 3.6V / aus VCC < 2.6V
// PWM1 = Schalter ein ~ VCC > 3.0V / aus VCC < 3.0V
// 
// Eine Solarzelle laed einen Superkondensator auf, der ueber eine Diode gegen Entladung geschuetzt wird. 
// Der ATtiny13A ueberwacht den Ladezustand des Superkondensators und schaltet einen Verbraucher ein und aus. 
// Wenn der Kondensator auf 3.6V geladen ist, dann wird der Verbraucher eingeschaltet und der Kondensator entladen.
// Wenn der Kondensator unter 2.6V entladen wird, dann wird der Verbraucher ausgeschaltet und der Kondensator geladen.
// Bei 3.0V Schaltet ein zweiter Schalter ein/aus und kann als LowPower-Signal vom Verbraucher genutzt werden.
// 
// HARDWARE:
// 1x ATtiny13a (Controller)
// 1x Diode N4001 (Entladeschutz)  VCC---|<|---(+)
// 1x SuperCap 0.47F 5.5 (Akku)    VCC---| |---GND
// 1x LED 1.6V 4mA (VCC)           GND---|<|--+[R1]/[R2]
// 1x Widerstand-1 1 MOhm (VCC)    VCC---[R1]-+[R2]/LED
// 1x Widerstand-2 10 KOhm (LED)   ADC4--[R2]-+[R1]/LED
// 
// SCHALTUNG:
//  +-----------|0.47F|------------+
//  +  +---------------------------+
//  | [1M]     [RST   ATtiny  VCC]--|<|--(+) Solar
//  |  |-[10k]-[ADC3   13a   ADC1]
//  | \/   LED-[ADC4         PWM1]--CHARGE ab 3.0V
//  (-)--------[GND          PWM0]--SWITCH von 3.3V bis 2.7V
//
// WEBLINKS:
// readVCC:  arduino-1.6.8\hardware\tools\avr\avr\include\avr\iotnx5.h
// ATtiny13A 14.12.1 ADMUX – ADC Multiplexer Selection Register
// 
// BOARD: ATtiny13, 1MHz disable BrownOut
// PGM 1024 B | VAR 64 B | Bemerkung
//  270 (26%) |  9 (14%) | leer
//  412 (40%) |  9 (14%) | PinMode (Setup)
//  554 (54%) |  9 (14%) | DigitalWrite (Setup)
//  830 (81%) | 11 (17%) | getVCC
//  852 (83%) | 11 (17%) | setSWITCH
//  870 (84%) | 13 (20%) | if setSWITCH
//  894 (87%) | 13 (20%) | if getVCC
//  948 (92%) | 13 (20%) | setBlinken
//  994 (97%) | 13 (20%) | setPWM1

#define __AVR_ATtiny13A__
#define REFS0 6  // Reference Selection Bit = voltage reference for the ADC (0: Vcc, 1: Internal Voltage)
#define ADLAR 5  // ADC Left Adjust Result
#define MUX1  1  // Analog Channel Selection Bits
#define MUX0  0  // Analog Channel Selection Bits

#define DELAY_100MS 120  // 100 ms auf ATtiny13a 
#define DELAY_1S 1200    // 1 s auf ATtiny13a
#define DELAY_ADC 60     // 50 ms auf ATtiny13a

#define ADC_26V 6155  // readAdc() 2.6V
#define ADC_27V 5970  // readAdc() 2.7V
#define ADC_30V 5415  // readAdc() 3.0V
#define ADC_33V 4860  // readAdc() 3.3V
#define ADC_36V 4568  // readAdc() 3.6V
#define ADC_45V 3690  // readAdc() 4.5V

#define PIN_GETVCC 3  // ADC3=Pin3 Spannung der Solarzelle messen
#define PIN_SWITCH 0  // PWM0=Pin0 Schalter fuer Verbraucher
#define PIN_LED 4     // ADC4=Pin4 Signal-LED fuer Ladezustand
#define PIN_CHARGE 1  // PWM1=Pin1 Schalter fuer Lade

#define MODE_CHARGE 1   // aufladen auf 3.3V
#define MODE_POWERON 2  // entladen auf 2.7V

int lastADC = 0;  // letzte gemessene Spannung
byte setSWITCH = MODE_CHARGE;  // letzter Ausgabemodus

int readAdc() { 
  // http://www.avrfreaks.net/forum/please-big-help-attiny13-adc
  // 
  // Messbereich 2.7-3.3V
  // 541.5/1024 = 1.6V/3.0V mit 1.6V LED als Vref
  // 10x Messungen ~ 5415/1024 = 10*1.6V/3.0V
  // 
  // ADC <= 9350 ~ 1.3V
  // ADC <= 6155 ~ 2.6V
  // ADC <= 5970 ~ 2.7V
  // ADC <= 5415 ~ 3.0V
  // ADC <= 4860 ~ 3.3V
  // ADC <= 4568 ~ 3.6V
  // ADC <= 3690 ~ 4.5V
  // 
  ADCSRA |= (1 << ADEN);           // Enable ADC  
  ADMUX |= (0 << REFS0);           // vcc reference
  ADMUX = PIN_GETVCC;              // ADC3 PB.3
  for (byte i=0; i<10; i++) 
  {                                // ADC einschwingen
    ADCSRA |= (1 << ADSC);         // Start Converstion
    delay(DELAY_ADC);              // Wait for Vref to settle
    while((ADCSRA & 0x40) !=0){};  // wait for conv complete
  }
  int adcValue = 0;
  for (byte i=0; i<10; i++) 
  {                                // mehrere Messungen
    ADCSRA |= (1 << ADSC);         // Start Converstion
    delay(DELAY_ADC);              // Wait for Vref to settle
    while((ADCSRA & 0x40) !=0){};  // wait for conv complete
    adcValue = adcValue + ADC;
  }
  return adcValue;
}

void setup() {
  pinMode(PIN_SWITCH, OUTPUT);     // PIN0 = Schalter
  pinMode(PIN_LED, OUTPUT);        // PIN4 = LED
  pinMode(PIN_CHARGE, OUTPUT);     // PIN1 = Ladegeraet
  // digitalWrite(PIN_SWITCH, LOW);   // Schalter aus
  // digitalWrite(PIN_LED, LOW);      // LED aus
  // digitalWrite(PIN_CHARGE, LOW);   // Ladegeraet aus
}

void loop() {
  lastADC = readAdc();

  if (lastADC < ADC_36V)             // if GETVCC > 3.6V
    setSWITCH = MODE_POWERON;        // then PowerOn
  else if (lastADC > ADC_26V)        // if GETVCC < 2.6V
    setSWITCH = MODE_CHARGE;         // then Charge

  if (lastADC < ADC_30V)             // if GETVCC > 3.0V
    digitalWrite(PIN_CHARGE, HIGH);  // then Charge On
  else                               // else LowPower
    digitalWrite(PIN_CHARGE, LOW);   // then Charge Off

  if (setSWITCH >= MODE_POWERON)     // if Charged
    digitalWrite(PIN_SWITCH, HIGH);  // then PowerOn
  else                               // else Discharged
    digitalWrite(PIN_SWITCH, LOW);   // then PowerOff

  while((lastADC+=555)<6710) {
    digitalWrite(PIN_LED, HIGH);     // LED ein
    delay(DELAY_100MS);
    digitalWrite(PIN_LED, LOW);      // LED aus
    delay(DELAY_100MS);
  }

  delay(DELAY_1S);
}
