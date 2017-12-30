#ifndef InterruptADC_h
#define InterruptADC_h

#include "Arduino.h"

class InterruptADC {
  public:
    InterruptADC(uint8_t channels);
    void begin();
    int read(byte channel);
    void isr();
  private:
    int *_values;
    uint8_t _channel;
    uint8_t _channels;
    uint8_t _sample;
};

#endif
