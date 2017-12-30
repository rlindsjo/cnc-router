#include <U8g2lib.h>

#include "InterruptADC.h"

// 16 MHz

// For tracking analog in
InterruptADC adc(3);

// Six digital outputs (step and direction of x, y and z)
static uint8_t motorStates;
byte x = 255;

byte xSpeed = 255;
byte ySpeed = 255;
byte zSpeed = 255;

byte xActual = 255;
byte yActual = 255;
byte zActual = 255;

const byte MAX_STEPS = 40;
byte step;

// Start in stopped state
byte xStep = 255;
byte yStep = 255;
byte zStep = 255;
byte toggle2 = 0;

// TWI I2C (1 pins) for display
// Analog 4 (27)
U8G2_SH1106_128X64_NONAME_1_HW_I2C oled(U8G2_R0);

void setup() {
  oled.begin();
  oled.setFont(u8g2_font_7x14_tf);
  displayWelcome();

  step = MAX_STEPS;

  // Set digital 8 - 13 for digital output (14-19)
  motorStates = 0;
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);

  // Set 2-7 for digital input (4-6, 11-13)
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);

  // Timer 2 to 8KHz
  TCCR2A = 0; // set entire TCCR2A register to 0
  TCCR2B = 0; // same for TCCR2B
  TCNT2  = 0; //initialize counter value to 0
  // set compare match register for 8khz increments
  OCR2A = 61; // = (16*10^6) / (8000*32) - 1 (must be <256)
  TCCR2A |= (1 << WGM21); // turn on CTC mode
  TCCR2B |= (1 << CS21); // Set CS21 bit for 8 prescaler
  TIMSK2 |= (1 << OCIE2A); // enable timer compare interrupt

  adc.begin();

  // Enable interrupts
  sei();

  calibrate();
}

uint8_t max_speed = 3;

void loop() {
  int t = (adc.read(0) - 512) >> 1;
  if (t > 0) {
    motorStates |= B00001000;
  } else {
    motorStates &= B11110111;
  }
  x = 256 - abs(t);
  int y = adc.read(1) - 512;
  int z = adc.read(2) - 512;

  readEncoders();
  displayPos(x, y, z);
}

void displayWelcome() {
  oled.firstPage();
  do {
    oled.setCursor(0, 10);
    oled.print("Calibrating");
    oled.setCursor(0, 25);
    oled.print("Please wait");
    oled.setCursor(0, 64);
    oled.print("Tilialacus CNC");
  } while (oled.nextPage());
}

void displayPos(int x, int y, int z) {
  char buf[5]; // Only 3 digit signed numbers
  uint8_t offset, line;
  oled.firstPage();
  do {
    line = 10;
    offset = oled.drawStr(0, line, "x=");
    offset += oled.drawStr(offset, line, itoa(x, buf, 10));
    offset = 64;
    offset += oled.drawStr(offset, line, "y=");
    oled.drawStr(offset, line, itoa(y, buf, 10));
    line += 15;
    offset = 0;
    offset = oled.drawStr(0, line, "z=");
    offset += oled.drawStr(offset, line, itoa(z, buf, 10));
    offset = 64;
    offset += oled.drawStr(offset, line, "s=");
    oled.drawStr(offset, line, itoa(max_speed, buf, 10));
    oled.drawStr(0, 64, "Tilialacus CNC");
  } while (oled.nextPage());
}

void readEncoders() {
  byte input = PIND;
  if (input & B01000000) {
    max_speed = 8;
  } else {
    max_speed = 3;
  }
}

uint8_t s = 0;
uint8_t dx = 255;
uint8_t wx = 255;

ISR(TIMER2_COMPA_vect){
  if (x < 250) {
    if (--dx == 0) {
      motorStates ^= B00000001;
      dx = wx;
    }
  }
  if (++s == 255) {
    if (wx > x) {
      if (wx <= max_speed) {
        wx = max_speed;
      } else {
        wx -= (((wx - x) >> 4) + 1);
      }
    } else {
      wx = x;
    }
  }
  PORTB = motorStates;
}

void calibrate() {
  delay(1000); // Placeholder to give time to calibrate system;
}
