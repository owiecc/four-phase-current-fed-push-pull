
#include "relay.h"
#include "f28x_project.h"

void relayOn(void)
{
    GpioDataRegs.GPBSET.bit.GPIO34 = 1;
}

void relayOff(void)
{
    GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;
}
