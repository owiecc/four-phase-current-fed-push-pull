
#include "input.h"
#include "f28x_project.h"

button button_pressed(void)
{
    if (GpioDataRegs.GPBDAT.bit.GPIO44 == 1) { return BtnOff; }; // Off
    if (GpioDataRegs.GPBDAT.bit.GPIO45 == 1) { return BtnReset; }; // Reset
    if (GpioDataRegs.GPBDAT.bit.GPIO46 == 1) { return BtnOn; }; // On
    if (GpioDataRegs.GPBDAT.bit.GPIO42 == 1) { return BtnZero; }; // Reference zero
    if (GpioDataRegs.GPBDAT.bit.GPIO39 == 1) { return BtnDecr; }; // Reference decrease
    if (GpioDataRegs.GPBDAT.bit.GPIO43 == 1) { return BtnIncr; }; // Reference increase
    return BtnNothing;
}
