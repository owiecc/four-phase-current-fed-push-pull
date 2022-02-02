
#include "controller.h"
#include "f28x_project.h"

// adcA1ISR - ADC A Interrupt 1 ISR
__interrupt void adcA1ISR(void)
{
    // GpioDataRegs.GPATOGGLE.bit.GPIO22 = 1;
    // ADCRESULT0 is the result register of SOC0
    // AdcaResultRegs.ADCRESULT0;

    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // Clear the interrupt flag

    // Check if overflow has occurred
    if(1 == AdcaRegs.ADCINTOVF.bit.ADCINT1)
    {
        AdcaRegs.ADCINTOVFCLR.bit.ADCINT1 = 1; //clear INT1 overflow flag
        AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // Acknowledge the interrupt
}
