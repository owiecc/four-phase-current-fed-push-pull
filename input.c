
#include "input.h"
#include "f28x_project.h"

button button_pressed(void)
{
    if (GpioDataRegs.GPBDAT.bit.GPIO44 == 0) { return BtnOff; }; // Off
    if (GpioDataRegs.GPBDAT.bit.GPIO45 == 0) { return BtnClrTrip; }; // Clear trip
    if (GpioDataRegs.GPBDAT.bit.GPIO46 == 0) { return BtnOn; }; // On
    if (GpioDataRegs.GPBDAT.bit.GPIO42 == 0) { return BtnZero; }; // Reference zero
    if (GpioDataRegs.GPBDAT.bit.GPIO39 == 0) { return BtnDecr; }; // Reference decrease
    if (GpioDataRegs.GPBDAT.bit.GPIO43 == 0) { return BtnIncr; }; // Reference increase
    return BtnNothing;
}
