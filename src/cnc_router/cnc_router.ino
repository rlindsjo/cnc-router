#include <U8g2lib.h>

#include "InterruptADC.h"

// 16 MHz

// For tracking analog in
InterruptADC adc(3);

uint8_t count = 0;

uint8_t max_speed = 3;

struct Motor {
  uint8_t pins;
  int32_t pos;
  int32_t target_pos;
  uint8_t step;
  uint8_t actual;
  uint8_t target;
  bool dir;
  bool on;
};

struct Motor x_motor = { B00001001, 0, 0x7FFFFF ,0, 255, 255, 0, 0};
struct Motor y_motor = { B00010010, 0, 0x7FFFFF, 0, 255, 255, 0, 0};
struct Motor z_motor = { B00100100, 0, 0, 0, 255, 255, 0, 0};
uint8_t motor_correction = B00101000;

// TWI I2C (1 pins) for display
// Analog 4 (27)
U8G2_SH1106_128X64_NONAME_1_HW_I2C oled(U8G2_R0);

void setup() {
  oled.begin();
  oled.setFont(u8g2_font_7x14_tf);
  displayWelcome();

  // Set digital 8 - 13 for digital output (14-19)
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);

  // Set 2-7 for digital input (4-6, 11-13)
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
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

volatile bool enable_joystick = 1;
void loop() {
  readEncoders();
  displayPos(&x_motor, &y_motor, &z_motor);
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

void displayPos(Motor * x, Motor * y, Motor * z) {
  char buf[12];
  uint8_t offset, line;
  oled.firstPage();
  do {
    line = 10;
    offset = oled.drawStr(0, line, "x=");
    offset += oled.drawStr(offset, line, itoa(x->pos, buf, 10));
    offset = 64;
    offset += oled.drawStr(offset, line, "y=");
    oled.drawStr(offset, line, itoa(y->pos, buf, 10));
    line += 15;
    offset = 0;
    offset = oled.drawStr(0, line, "z=");
    offset += oled.drawStr(offset, line, itoa(z->pos, buf, 10));
    offset = 64;
    offset += oled.drawStr(offset, line, "s=");
    line += 15;
    offset = 0;
    offset = oled.drawStr(0, line, "x=");
    offset += oled.drawStr(offset, line, itoa(x->target_pos, buf, 10));
    offset = 64;
    offset += oled.drawStr(offset, line, "y=");
    oled.drawStr(offset, line, itoa(y->target_pos, buf, 10));
    line += 15;
    offset = 0;
    offset = oled.drawStr(0, line, "z=");
    offset += oled.drawStr(offset, line, itoa(z->target_pos, buf, 10));
    oled.drawStr(offset, line, itoa(count, buf, 10));
    oled.drawStr(0, 64, "Tilialacus CNC");
  } while (oled.nextPage());
}

void readEncoders() {
  byte input = PIND;

  rotary((PIND >> 1) & B11);

  if (input & B01000000 == 0) {
    reset(&x_motor);
    reset(&y_motor);
    reset(&z_motor);
  }
}

volatile uint8_t pins_old;

void rotary(uint8_t pins) {
  if ((pins & B10) != (pins_old & B10)) {
    if (pins == B11 || pins == B00) {
      z_motor.target_pos += 50;
    } else {
      z_motor.target_pos -= 50;
    }
    z_motor.target = 0;
    z_motor.dir = z_motor.target_pos > z_motor.pos;
    pins_old = pins;
  }
}

uint8_t reset(volatile struct Motor * m) {
  m->pos = 0;
  m->target_pos = 0;
  m->target = 255;
}

uint8_t timer_step = 0;

ISR(TIMER2_COMPA_vect){
  readEncoders();
  if (++timer_step == 0 && enable_joystick) {
    updateMotor(&x_motor, adc.read(0));
    updateMotor(&y_motor, adc.read(1));
  }
  uint8_t motor_states =
    (calculateMotor(&x_motor)
    | calculateMotor(&y_motor)
    | calculateMotor(&z_motor))
    ^ motor_correction;
  PORTB = motor_states;
}

uint8_t updateMotor(volatile struct Motor * m, int16_t reading) {
  reading = (reading - 512) >> 1;
  m->dir = reading > 0;
  m->target = 256 - abs(reading);
}

uint8_t calculateMotor(volatile struct Motor * m) {
  if (m->pos == m->target_pos) {
    return 0;
  }
  if (m->target < 230) {
    if (--m->step == 0) {
      m->on = !m->on;
      m->step = m->actual;
      if (m->dir) {
        ++m->pos;
      } else {
        --m->pos;
      }
    }
  }
  if (timer_step == 255) {
    if (m->actual > m->target) {
      if (m->actual <= max_speed) {
        m->actual = max_speed;
      } else {
        m->actual -= (((m->actual - m->target) >> 4) + 1);
      }
    } else {
      m->actual = m->target;
    }
  }
  return ((m->dir ? B00111000 : 0) | (m->on ? B00000111 : 0)) & m->pins;
}

void calibrate() {
  delay(1000); // Placeholder to give time to calibrate system;
}
