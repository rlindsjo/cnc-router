#include <U8g2lib.h>

#include "InterruptADC.h"

// 16 MHz

// For tracking analog in
InterruptADC adc(3);

// Six digital outputs (step and direction of x, y and z)
byte motorStates;

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
byte c = 0;

// Six digital inputs for rotary encoders for x, y, and z)

// TWI I2C (1 pins) for display
// Analog 4 (27)
U8G2_SH1106_128X64_NONAME_1_HW_I2C oled(U8G2_R0);

void setup() {
  // Set PC0-PC2 as analog inputs

  //  calibrate();

  // Set digital 8 - 13 for digital output (14-19)
  step = MAX_STEPS;
  motorStates = 0;
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);

  // Set 2-7 for digital input (4-6, 11-13)
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(7, INPUT);

  adc.begin();
  // I2C
  oled.begin();
  oled.setFont(u8g2_font_7x14_tf);
  displayWelcome();
}

void loop() {
  int x = adc.read(0);
  int y = adc.read(1);
  int z = adc.read(2);
  int s = adc.read(6);

  oled.firstPage();
  do {
    char buf[12];
    uint8_t offset, line;
    line = 10;
    itoa(x, buf, 10);
    offset = oled.drawStr(0, line, "x=");
    offset += oled.drawStr(offset, line, buf);
    itoa(y, buf, 10);
    offset = 64;
    offset += oled.drawStr(offset, line, "y=");
    oled.drawStr(offset, line, buf);
    line += 15;
    offset = 0;
    itoa(z, buf, 10);
    offset = oled.drawStr(0, line, "z=");
    offset += oled.drawStr(offset, line, buf);
    oled.drawStr(0, 64, "Tilialacus CNC");
  } while (oled.nextPage());
}

void displayWelcome() {
  oled.firstPage();
  do {
    oled.setCursor(0, 25);
    oled.print("Tilialacus CNC");
  } while (oled.nextPage());
}

void readEncoders() {
  byte input = PORTD;
  // Do stuff using bit 2-7
}

void stepMotors() {
  // Calculate state of each motor
  if (--step == 0) {
    step = MAX_STEPS;
  }

  if (xSpeed > 0) {
    if (xStep == xSpeed) {
      // Toggle x
      motorStates ^= 0b00000001;
      xStep = 0;
    } else {
      ++xStep;
    }
  }
  PORTB = motorStates;
}

void readJoystick() {
  int read = adc.read(0);
  if (read > 512) {
    motorStates |= 0b00001000;
    read = 1023 - read;
  } else {
    motorStates &= 0b11110111;
  }

  if (read > 230) {
    xSpeed = 0;
  } else {
    xSpeed = 1;
    for (int i = 0; i < read; i += 25) {
      ++xSpeed;
    }
  }
}

void calibrate() {
}

void sendDisplay() {
}


