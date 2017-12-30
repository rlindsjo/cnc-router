#include "InterruptADC.h"

static InterruptADC * _instance;

InterruptADC::InterruptADC(uint8_t channels) {
  _values = new int[channels];
  _channels = channels;
}

void InterruptADC::begin() {
  _instance = this;
  _channel = 0;
  _sample = 0;
  ADMUX = 0x40;            
  ADCSRA = 0xAC; // AD-converter on, interrupt enabled, prescaler = 128
  ADCSRB = 0x40; // AD channels MUX on, free running mode
  ADCSRA |= 1 << 6;  // Start the conversion
}

void InterruptADC::isr() {
  if (++_sample < 5) {
    return;
  }
  
  uint16_t aval = ADCL; // lower uint8_t
  aval += ADCH << 8;  // higher uint8_t
  _values[_channel] = aval;
  if (++_channel == _channels) {
    _channel = 0;
  }
  ADMUX = 0x40 | _channel;
  _sample = 0;
}

int InterruptADC::read(uint8_t channel) {
  return _values[channel];
}

ISR(ADC_vect) {
  _instance->isr();
}


