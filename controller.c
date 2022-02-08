
#include "controller.h"
#include "f28x_project.h"

struct SOALimitsConverter SOA = {
    .Vin =    (struct SOALimits) {.maxTrip =  840, .maxStartup =  820, .minStartup =  780, .minTrip = 720 },
    .Vout =   (struct SOALimits) {.maxTrip =  930, .maxStartup =  925, .minStartup =  195, .minTrip = 190 },
    .Vclamp = (struct SOALimits) {.maxTrip = 1260, .maxStartup = 1230, .minStartup = 1170, .minTrip = 1140},
    .Iout =   (struct SOALimits) {.maxTrip =    2, .maxStartup =  0.1, .minStartup = -0.1, .minTrip = 0   }
};

inline int isInSOATrip(float val, struct SOALimits lims)
{
    return val > lims.maxTrip + val < lims.minTrip;
}

inline int isInSOAStartup(float val, struct SOALimits lims)
{
    return val > lims.maxStartup + val < lims.minStartup;
}

int isInSOA(struct ADCResult sensors, enum converter_states cs)
{
    switch (cs) {
    case StateOn:
        if (isInSOATrip(sensors.Iout, SOA.Iout)) return 0;
        if (isInSOATrip(sensors.Vclamp, SOA.Vclamp)) return 0;
        if (isInSOATrip(sensors.Vout, SOA.Vout)) return 0;
        if (isInSOATrip(sensors.Vin, SOA.Vin)) return 0;
        break;
    default:
        if (isInSOAStartup(sensors.Iout, SOA.Iout)) return 0;
        if (isInSOAStartup(sensors.Vclamp, SOA.Vclamp)) return 0;
        if (isInSOAStartup(sensors.Vout, SOA.Vout)) return 0;
        if (isInSOAStartup(sensors.Vin, SOA.Vin)) return 0;
    }
    return 0;
}

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
