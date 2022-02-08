
#include "relay.h"
#include "f28x_project.h"

void relayOn(void)
{
    GpioDataRegs.GPASET.bit.GPIO34 = 1;
}

void relayOff(void)
{
    GpioDataRegs.GPACLEAR.bit.GPIO34 = 1;
}
