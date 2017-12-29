#ifndef InterruptADC_h
#define InterruptADC_h

#include "Arduino.h"

class InterruptADC {
  public:
    InterruptADC(uint8_t channels);
    void begin();
    int read(byte channel);
  private:
};

#endif
