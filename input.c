
#include "input.h"
#include "f28x_project.h"

button button_pressed(void)
{
    if (GpioDataRegs.GPBDAT.bit.GPIO48 == 0) { return BtnOff; }; // Off
    if (GpioDataRegs.GPBDAT.bit.GPIO50 == 0) { return BtnClrTrip; }; // Clear trip
    if (GpioDataRegs.GPBDAT.bit.GPIO52 == 0) { return BtnOn; }; // On
    if (GpioDataRegs.GPBDAT.bit.GPIO49 == 0) { return BtnZero; }; // Reference zero
    if (GpioDataRegs.GPBDAT.bit.GPIO40 == 0) { return BtnDecr; }; // Reference decrease
    if (GpioDataRegs.GPBDAT.bit.GPIO51 == 0) { return BtnIncr; }; // Reference increase
    return BtnNothing;
}
