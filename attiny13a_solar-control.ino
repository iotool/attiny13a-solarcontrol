// ATtiny13a SolarControl
// 
// ADC3 = VCC der Solarzelle ueber 1.6V LED als Vref messen
// ADC4 = optionale LED mit Blinkcode fuer Spannung (2.6/3.0/3.6/4.2V)
// PWM0 = Schalter ein ~ VCC > 3.6V / aus VCC < 2.6V
// PWM1 = Schalter ein ~ VCC > 3.0V / aus VCC < 3.0V
// 
// Eine Solarzelle laed einen Superkondensator auf, der ueber eine Diode gegen Entladung geschuetzt wird. 
// Der ATtiny13A ueberwacht den Ladezustand des Superkondensators und schaltet einen Verbraucher ein und aus. 
// Wenn der Kondensator auf 3.6V geladen ist, dann wird der Verbraucher eingeschaltet und der Kondensator entladen.
// Wenn der Kondensator unter 2.6V entladen wird, dann wird der Verbraucher ausgeschaltet und der Kondensator geladen.
// Bei 3.0V Schaltet ein zweiter Schalter ein/aus und kann als LowPower-Signal vom Verbraucher genutzt werden.
// 
// Ladezeit bei Bewoelkung von 0.8V auf 3.6V ca. 5 Minuten fuer 0.47F 5.5V Supercap mit Solar 6V 110x60mm
// 0.47F von 3.6V bis 2.6V entsprechen 470mAs
// 
// HARDWARE:
// 1x ATtiny13a (Controller)
// 1x Diode N4001 (Entladeschutz)  VCC---|<|---(+)
// 1x SuperCap 0.47F 5.5V (Akku)   VCC---| |---GND
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
// HISTORIE:
// 2017-03-21(V1)  adjust ADC-VCC 2.6V / 3.7V
// 2017-03-22(V3)  sleep, low level api, reduce bytecode
// 
// BOARD: ATtiny13, 1MHz disable BrownOut
// PGM 1024 B | VAR 64 B | Bemerkung
//  390 (38%) |  1 (1%)  | low level api

#define _AVR_ATTINY13A_H_ 1  // iotn13a.h
#define F_CPU 1200000UL      // delay cpu clock
#include <avr/io.h>          // low level pinmode
#include <avr/interrupt.h>   // timer interrupt for sleep
#include <avr/sleep.h>       // sleep to reduce power
#include <util/delay.h>      // low level delay

// interrupt timer
#define SLEEPTIMER_16MS   WDTCR |= (0<<WDP3)|(0<<WDP2)|(0<<WDP1)|(0<<WDP0)
#define SLEEPTIMER_32MS   WDTCR |= (0<<WDP3)|(0<<WDP2)|(0<<WDP1)|(1<<WDP0)
#define SLEEPTIMER_64MS   WDTCR |= (0<<WDP3)|(0<<WDP2)|(1<<WDP1)|(0<<WDP0)
#define SLEEPTIMER_125MS  WDTCR |= (0<<WDP3)|(0<<WDP2)|(1<<WDP1)|(1<<WDP0)
#define SLEEPTIMER_250MS  WDTCR |= (0<<WDP3)|(1<<WDP2)|(0<<WDP1)|(0<<WDP0)
#define SLEEPTIMER_500MS  WDTCR |= (0<<WDP3)|(1<<WDP2)|(0<<WDP1)|(1<<WDP0)
#define SLEEPTIMER_1S     WDTCR |= (0<<WDP3)|(1<<WDP2)|(1<<WDP1)|(0<<WDP0)
#define SLEEPTIMER_2S     WDTCR |= (0<<WDP3)|(1<<WDP2)|(1<<WDP1)|(1<<WDP0)
#define SLEEPTIMER_4S     WDTCR |= (1<<WDP3)|(0<<WDP2)|(0<<WDP1)|(0<<WDP0)
#define SLEEPTIMER_8S     WDTCR |= (1<<WDP3)|(0<<WDP2)|(0<<WDP1)|(1<<WDP0)
#define SLEEPTIMER_START  WDTCR |= (1<<WDTIE); WDTCR |= (0<<WDE); sei(); set_sleep_mode(SLEEP_MODE_PWR_DOWN)

// delay
#define DELAY_10MS   _delay_ms(12)
#define DELAY_50MS   _delay_ms(60)
#define DELAY_100MS  _delay_ms(120)
#define DELAY_1S     _delay_ms(1200)
#define DELAY_ADC    _delay_ms(6)
#define DELAY_MORSE  _delay_ms(120)

// pinMode(PBx,INPUT/OUTPUT)
#define PINMODE_PB0_INPUT  DDRB &= ~(1 << PB0)
#define PINMODE_PB1_INPUT  DDRB &= ~(1 << PB1)
#define PINMODE_PB2_INPUT  DDRB &= ~(1 << PB2)
#define PINMODE_PB3_INPUT  DDRB &= ~(1 << PB3)
#define PINMODE_PB4_INPUT  DDRB &= ~(1 << PB4)
#define PINMODE_PB0_OUTPUT  DDRB |= (1 << PB0)
#define PINMODE_PB1_OUTPUT  DDRB |= (1 << PB1)
#define PINMODE_PB2_OUTPUT  DDRB |= (1 << PB2)
#define PINMODE_PB3_OUTPUT  DDRB |= (1 << PB3)
#define PINMODE_PB4_OUTPUT  DDRB |= (1 << PB4)

// digitalWrite(PBx,LOW/HIGH)
#define DIGITALWRITE_PB0_LOW  PORTB &= ~(1 << PB0)
#define DIGITALWRITE_PB1_LOW  PORTB &= ~(1 << PB1)
#define DIGITALWRITE_PB2_LOW  PORTB &= ~(1 << PB2)
#define DIGITALWRITE_PB3_LOW  PORTB &= ~(1 << PB3)
#define DIGITALWRITE_PB4_LOW  PORTB &= ~(1 << PB4)
#define DIGITALWRITE_PB0_HIGH  PORTB |= (1 << PB0)
#define DIGITALWRITE_PB1_HIGH  PORTB |= (1 << PB1)
#define DIGITALWRITE_PB2_HIGH  PORTB |= (1 << PB2)
#define DIGITALWRITE_PB3_HIGH  PORTB |= (1 << PB3)
#define DIGITALWRITE_PB4_HIGH  PORTB |= (1 << PB4)

// digitalRead(PBx) == LOW
#define DIGITALREAD_PB0_LOW    (PINB & (1 << PB0))
#define DIGITALREAD_PB1_LOW    (PINB & (1 << PB1))
#define DIGITALREAD_PB2_LOW    (PINB & (1 << PB2))
#define DIGITALREAD_PB3_LOW    (PINB & (1 << PB3))
#define DIGITALREAD_PB4_LOW    (PINB & (1 << PB4))

// analog digital converter
#define ADC_DISABLE       ADCSRA &= ~(1 << ADEN)
#define ADC_ENABLE        ADCSRA |= (1 << ADEN); ADCSRA |= (1 << ADSC); while (ADCSRA & (1 << ADSC))
#define ADC1_PB2_SELECT   ADMUX |= (0 << REFS0); ADMUX |= (0 << MUX1)|(1 << MUX0)
#define ADC2_PB4_SELECT   ADMUX |= (0 << REFS0); ADMUX |= (1 << MUX1)|(0 << MUX0)
#define ADC3_PB3_SELECT   ADMUX |= (0 << REFS0); ADMUX |= (1 << MUX1)|(1 << MUX0)
#define ADC_PREREAD       5
#define ADC_READ          10

// mapping readAdc() voltage
#define ADC_26V 6240 // readAdc() 2.6V (2.61V)
#define ADC_27V 5995 // readAdc() 2.7V
#define ADC_30V 5430 // readAdc() 3.0V (3.05V)
#define ADC_32V 5145 // readAdc() 3.2V
#define ADC_33V 5000 // readAdc() 3.3V (3.65V)
#define ADC_36V 4600 // readAdc() 3.6V
#define ADC_42V 4000 // readAdc() 4.2V
#define ADC_45V 3727 // readAdc() 4.5V
#define ADC_51V 3340 // readAdc() 5.1V 

// void setup() {}  // Use main() instat of setup()/loop()
// void loop() {}   // to reduce bytecode from 270 to 44 byte

void morseNumber(byte a, byte b, byte c, byte d, byte e) {
  if (a <= 1) {DIGITALWRITE_PB4_HIGH; DELAY_MORSE; if (a >= 1) DELAY_MORSE; DIGITALWRITE_PB4_LOW; DELAY_MORSE;}
  if (b <= 1) {DIGITALWRITE_PB4_HIGH; DELAY_MORSE; if (b >= 1) DELAY_MORSE; DIGITALWRITE_PB4_LOW; DELAY_MORSE;}
  if (c <= 1) {DIGITALWRITE_PB4_HIGH; DELAY_MORSE; if (c >= 1) DELAY_MORSE; DIGITALWRITE_PB4_LOW; DELAY_MORSE;}
  if (d <= 1) {DIGITALWRITE_PB4_HIGH; DELAY_MORSE; if (d >= 1) DELAY_MORSE; DIGITALWRITE_PB4_LOW; DELAY_MORSE;}
  if (e <= 1) {DIGITALWRITE_PB4_HIGH; DELAY_MORSE; if (e >= 1) DELAY_MORSE; DIGITALWRITE_PB4_LOW; DELAY_MORSE;}
  DELAY_MORSE; DELAY_MORSE; 
}

void morseDigit(byte digit) {
  if (digit == 0) morseNumber(1,1,1,1,1); // 0
  if (digit == 1) morseNumber(0,1,1,1,1); // 1
  if (digit == 2) morseNumber(0,0,1,1,1); // 2
  if (digit == 3) morseNumber(0,0,0,1,1); // 3
  if (digit == 4) morseNumber(0,0,0,0,1); // 4
  if (digit == 5) morseNumber(0,0,0,0,0); // 5
  if (digit == 6) morseNumber(1,0,0,0,0); // 6
  if (digit == 7) morseNumber(1,1,0,0,0); // 7
  if (digit == 8) morseNumber(1,1,1,0,0); // 8
  if (digit == 9) morseNumber(1,1,1,1,0); // 9  
}

void morseCode(int value) {
  byte morse_1000 = 0;
  byte morse_100 = 0;
  byte morse_10 = 0;
  byte morse_1 = 0;
  while(value >= 1000) {
    morse_1000++;
    value-=1000;
  }
  while(value >= 100) {
    morse_100++;
    value-=100;
  }
  while(value >= 10) {
    morse_10++;
    value-=10;
  }
  morse_1 = value;
  morseDigit(morse_1000);
  morseDigit(morse_100);
  morseDigit(morse_10);
  morseDigit(morse_1);
  DELAY_MORSE; DELAY_MORSE; DELAY_MORSE; 
}

int readADC() {
  byte i;
  for (i=0; i<ADC_PREREAD; i++) {  // ADC einschwingen
    ADCSRA |= (1 << ADSC);         // start converstion
    DELAY_ADC;                     // wait for Vref to settle
    while((ADCSRA & 0x40) !=0){};  // wait for conv complete
  }
  int sumAdc = 0;
  for (byte i=0; i<ADC_READ; i++) 
  {                                // ADC Messungen
    ADCSRA |= (1 << ADSC);         // start converstion
    DELAY_ADC;                     // wait for Vref to settle
    while((ADCSRA & 0x40) !=0){};  // wait for conv complete
    sumAdc += ADC;
  }
  return sumAdc;
}

byte ticker = 0;    // count 64 ms sleep interval

ISR(WDT_vect) {
  ticker++;
}

int main() {

  // setup - once

  PINMODE_PB0_OUTPUT;
  PINMODE_PB1_OUTPUT;
  PINMODE_PB2_OUTPUT;
  PINMODE_PB3_OUTPUT;
  PINMODE_PB4_OUTPUT;

  SLEEPTIMER_64MS;
  SLEEPTIMER_START;
  ADC_DISABLE;

  // loop

  int lastVCC = 0;
  ticker = 0;

  while(1) {
    // endless loop with timer based slots
    // ticker increments every 64ms by interrupt
    switch(ticker) {
      case 1:
        // messure vcc
        PINMODE_PB3_INPUT;        // use PB3 as input
        ADC3_PB3_SELECT;          // use ADC3 for read
        ADC_ENABLE;               // enable ADC hardware
        lastVCC = readADC();      // get VCC by know Vref of LED
        ADC_DISABLE;              // reduce power
        PINMODE_PB3_OUTPUT;       // reduce power
        // debug vcc
        // morseCode(lastVCC);
        // switch output
        if (lastVCC < ADC_30V) { 
          DIGITALWRITE_PB1_HIGH;  // VCC > 3.0V = switch on
          ticker = 6;             // 2x blink code (next ticker 7)
        } else {
          DIGITALWRITE_PB1_LOW;   // VCC < 3.0V = switch off
          ticker = 9;             // 1x blink code (next ticker 10)
        }
        if (lastVCC < ADC_36V) { 
          DIGITALWRITE_PB0_HIGH;  // VCC > 3.6V = switch on
          ticker = 3;             // 3x blink code (next ticker 4)
        } else if (lastVCC > ADC_26V) {
          DIGITALWRITE_PB0_LOW;  // VCC < 2.6V = switch off
          ticker = 12;           // blink disable (next ticker 13)
        }
        if (lastVCC < ADC_42V) { // VCC > 4.2V
          ticker = 1;            // 4x blink code (next ticker 2)
        }
        break;
      case  2: DIGITALWRITE_PB4_HIGH; break;  case  3: DIGITALWRITE_PB4_LOW; break;
      case  5: DIGITALWRITE_PB4_HIGH; break;  case  6: DIGITALWRITE_PB4_LOW; break;
      case  8: DIGITALWRITE_PB4_HIGH; break;  case  9: DIGITALWRITE_PB4_LOW; break;
      case 11: DIGITALWRITE_PB4_HIGH; break;  case 12: DIGITALWRITE_PB4_LOW; break;
      case 14:
        if (DIGITALREAD_PB3_LOW) {} 
        else {                   // fast refresh VCC every second (because switch is on)
          ticker = 31;           // 32x 64ms = 2048 ms left until next ticker overflow
        }
        break;
      default:
        if (ticker >= 63)        // refresh VCC after 4032 ms sleep
          ticker = 0;            // ticker overflow every 2.5 seconds
        break;
    }
    sleep_mode();                // power down sleep mode (0.007 mA)
  }
  
}

