#include "InterruptADC.h"

static int *_values;
static uint8_t _channel;
static uint8_t _channels;
static uint8_t _sample;

InterruptADC::InterruptADC(uint8_t channels) {
  _values = new int[channels];
  _channels = channels;
}

void InterruptADC::begin() {
  _channel = 0;
  ADMUX = 0x40;            
  ADCSRA = 0xAC; // AD-converter on, interrupt enabled, prescaler = 128
  ADCSRB = 0x40; // AD channels MUX on, free running mode
  ADCSRA |= 1 << 6;  // Start the conversion
  sei();
}

ISR(ADC_vect) {
  if (++_sample < 3) {
    return;
  }
  
  int aval = ADCL; // lower uint8_t
  aval += ADCH << 8;  // higher uint8_t
  _values[_channel] = aval;
  if (++_channel == 3) {
    _channel = 0;
  }
  ADMUX = 0x40 | _channel;
  _sample = 0;
}

int InterruptADC::read(uint8_t channel) {
  return _values[channel];
}


