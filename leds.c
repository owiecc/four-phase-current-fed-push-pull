
#include "leds.h"
#include "f28x_project.h"

void ledOn(enum leds led)
{
    switch(led) {
    case LEDTripOC: GpioDataRegs.GPASET.bit.GPIO29 = 1; break;
    case LEDTripSOAVin: GpioDataRegs.GPASET.bit.GPIO28 = 1; break;
    case LEDTripSOAVout: GpioDataRegs.GPASET.bit.GPIO23 = 1; break;
    case LEDTripSOAVclamp: GpioDataRegs.GPASET.bit.GPIO22 = 1; break;
    case LEDIoutAdjust: GpioDataRegs.GPASET.bit.GPIO30 = 1; break;
    case LEDVclampAdjust: GpioDataRegs.GPASET.bit.GPIO31 = 1; break;
    }
}

void ledOff(enum leds led)
{
    switch(led) {
    case LEDTripOC: GpioDataRegs.GPACLEAR.bit.GPIO29 = 1; break;
    case LEDTripSOAVin: GpioDataRegs.GPACLEAR.bit.GPIO28 = 1; break;
    case LEDTripSOAVout: GpioDataRegs.GPACLEAR.bit.GPIO23 = 1; break;
    case LEDTripSOAVclamp: GpioDataRegs.GPACLEAR.bit.GPIO22 = 1; break;
    case LEDIoutAdjust: GpioDataRegs.GPACLEAR.bit.GPIO30 = 1; break;
    case LEDVclampAdjust: GpioDataRegs.GPACLEAR.bit.GPIO31 = 1; break;
    }
}

void ledsOff()
{
    ledOff(LEDTripOC);
    ledOff(LEDTripSOAVin);
    ledOff(LEDTripSOAVout);
    ledOff(LEDTripSOAVclamp);
    ledOff(LEDIoutAdjust);
    ledOff(LEDVclampAdjust);
}
