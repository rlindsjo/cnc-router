#include <U8g2lib.h>

// 16 MHz

// Three analoge inputs (x, y, z)
const int xaxisIn = 0;
const int yaxisIn = 1;
const int zaxisIn = 2;

// Six digital outputs (step and direction of x, y and z)
byte motorStates;

byte xSpeed = 255;
byte ySpeed = 255;
byte zSpeed = 255;

const byte MAX_STEPS = 230;
byte step;
// Start in stopped state
byte xStep = 255;
byte yStep = 255;
byte zStep = 255;
byte c = 0;

// Six digital inputs for rotary encoders for x, y, and z)

// TWI I2C (1 pins) for display
// Analog 4 (27)
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0);

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

  // I2C
  oled.begin();
  oled.setFont(u8g2_font_7x14_tf);
}

void loop() {
  readEncoders();
  readJoystick();
  stepMotors(); 
}

void readEncoders() {
  byte input = PORTD;
  // Do stuff using bit 2-7
}

void stepMotors() {
  // Calculate state of each motor
  if (--step == 0) {
    xStep = xSpeed;
    yStep = ySpeed;
    step = MAX_STEPS;
    c++;
    if ((c & 7) == 7) {
      c = 0;
      oled.firstPage();
      do {
        oled.setCursor(0, 25);
        oled.print(xSpeed);
        oled.setCursor(30, 25);
        oled.print(ySpeed);
        oled.setCursor(60, 25);
        oled.print(zSpeed);
        oled.setCursor(0, 45);
        oled.print((motorStates & 0b00001000) ? '<' : '>');
        oled.setCursor(30, 45);
        oled.print((motorStates & 0b00010000) ? '<' : '>');
        if (xStep == 0) {
          // Toggle x
          motorStates ^= 0b00000001;
          xStep = xSpeed;    
        } else {
          --xStep;
        }
        if (yStep == 0) {
          // Toggle y
          motorStates ^= 0b00000010;
          yStep = ySpeed;    
        } else {
          --yStep;
        }
      } while(oled.nextPage());
    }
  }
  
  if (xStep == 0) {
    // Toggle x
    motorStates ^= 0b00000001;
    xStep = xSpeed;    
  } else {
    --xStep;
  }
  if (yStep == 0) {
    // Toggle y
    motorStates ^= 0b00000010;
    yStep = ySpeed;    
  } else {
    --yStep;
  }
/*  
 if (--zStep == 0) {
    // Toggle z
    motorStates ^= 0b00000100;
    zStep = zSpeed;    
  }
*/
  PORTB = motorStates;
}

void readJoystick() {
  // Lets not do this every time.
  // Interrupts?
  if (step & 1) {
      int read = analogRead(xaxisIn);
      if (read > 512) {
        motorStates |= 0b00001000;
        read = 1023 - read;
      } else {
        motorStates &= 0b11110111;
      }
      xSpeed = read >> 1;
  } else {
      int read = analogRead(yaxisIn);
      if (read > 512) {
        motorStates |= 0b00010000;
        read = 1023 - read;
      } else {
        motorStates &= 0b11101111;
      }
      ySpeed = read>>1;
  }
      /*
      read = analogRead(zaxisIn);
      if (read > 512) {
        motorStates |= 0b00001000;
        read = 1023 - read;
      } else {
        motorStates &= 0b11110111;
      }
      zSpeed = read;
      */
}

void calibrate() {
  // Read joystick multiple times to get a 0;
  for (int i = 0; i < 200; i++) {
    analogRead(xaxisIn);
    analogRead(yaxisIn);
    analogRead(zaxisIn);
  }
}

void sendDisplay() {
}

