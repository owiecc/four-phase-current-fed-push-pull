
#include "iir_filter.h"
#include "controller.h"
#include "f28x_project.h"

struct OPLimitsConverter SOA = {
    .Vin =    (struct OPLimits) {.tripHi =  840, .startupHi =  820, .startupLo =  780, .tripLo = 720 },
    .Vout =   (struct OPLimits) {.tripHi =  930, .startupHi =  925, .startupLo =  195, .tripLo = 190 },
    .Vclamp = (struct OPLimits) {.tripHi = 1260, .startupHi = 1230, .startupLo = 1170, .tripLo = 1140},
    .Iout =   (struct OPLimits) {.tripHi =    2, .startupHi =  0.1, .startupLo = -0.1, .tripLo = 0   }
};

inline int isInSOAOn(float val, struct OPLimits lims)
{
    return val < lims.tripHi && val > lims.tripLo;
}

inline int isInSOAStartup(float val, struct OPLimits lims)
{
    return val < lims.startupHi && val > lims.startupLo;
}

enum trip_reasons isInSOA(struct ADCResult sensors, enum converter_states cs)
{
    switch (cs) {
    case StateOn:
        if (!isInSOAOn(sensors.Iout, SOA.Iout)) return TripOC;
        if (!isInSOAOn(sensors.Vclamp, SOA.Vclamp)) return TripSOAVclamp;
        if (!isInSOAOn(sensors.Vout, SOA.Vout)) return TripSOAVout;
        if (!isInSOAOn(sensors.Vin, SOA.Vin)) return TripSOAVin;
        break;
    case StateStandby:
        if (!isInSOAStartup(sensors.Iout, SOA.Iout)) return TripOC;
        if (!isInSOAStartup(sensors.Vclamp, SOA.Vout)) return TripSOAVclamp; // Vclamp is charged to Vout
        if (!isInSOAStartup(sensors.Vout, SOA.Vout)) return TripSOAVout;
        if (!isInSOAStartup(sensors.Vin, SOA.Vin)) return TripSOAVin;
        break;
    default:
        if (!isInSOAStartup(sensors.Iout, SOA.Iout)) return TripOC;
        if (!isInSOAStartup(sensors.Vclamp, SOA.Vclamp)) return TripSOAVclamp;
        if (!isInSOAStartup(sensors.Vout, SOA.Vout)) return TripSOAVout;
        if (!isInSOAStartup(sensors.Vin, SOA.Vin)) return TripSOAVin;
    }
    return NoTrip;
}

// adcA1ISR - ADC A Interrupt 1 ISR
__interrupt void adcA1ISR(void)
{
    // GpioDataRegs.GPATOGGLE.bit.GPIO22 = 1;
    // ADCRESULT0 is the result register of SOC0
    // AdcaResultRegs.ADCRESULT0;


    // static struct iirFilter Vclamp = {{6.3014,-11.4257,5.13},{1.0,-1.1429,0.1429},{0,0},{0,0}}; // init a d-axis PI controller
    // static struct iirFilter Iout = {{6.3014,-11.4257,5.13},{1.0,-1.1429,0.1429},{0,0},{0,0}}; // init a q-axis PI controller





    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // Clear the interrupt flag

    // Check if overflow has occurred
    if(1 == AdcaRegs.ADCINTOVF.bit.ADCINT1)
    {
        AdcaRegs.ADCINTOVFCLR.bit.ADCINT1 = 1; //clear INT1 overflow flag
        AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // Acknowledge the interrupt
}
