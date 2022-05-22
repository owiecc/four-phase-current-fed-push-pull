
#include "PI_controller.h"
#include "controller.h"
#include "f28x_project.h"
#include "pwm.h"

#define n1 14
#define n2 21
#define N ((float)n1/(float)n2)
#define Ninv ((float)n2/(float)n1)
#define FSW 40000
#define PI_Vc_Ki 0.05
#define PI_Io_Ki 0.10

static struct piController PI_Vc = {0, 0, 0, 0, 0}; // Vclamp controller
static struct piController PI_Io = {0, 0, 0, 0, 0}; // Iout controller

static float refIo = 0;

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

void initPIConttrollers(void)
{
    PI_Vc = initPI(PI_Vc_Ki/FSW, 2*PI_Vc_Ki/FSW, 0.5, -1, 0.99);
    PI_Io = initPI(PI_Io_Ki/FSW, 2*PI_Io_Ki/FSW, 0.5, -1, 0.00);
}

// adcA1ISR - ADC A Interrupt 1 ISR
__interrupt void adcA1ISR(void)
{
    // GpioDataRegs.GPATOGGLE.bit.GPIO22 = 1;

    struct ADCResult meas = scaleADCs();

    float deltaVclamp = 0;
    float errVclamp = meas.Vin*Ninv + deltaVclamp - meas.Vclamp;
    float errIout = refIo - meas.Iout;

    float d = updatePI(&PI_Vc, -errVclamp);
    float p = updatePI(&PI_Io, -errIout);

    updateModulator(d, p);

    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // Clear the interrupt flag

    // Check if overflow has occurred
    if(1 == AdcaRegs.ADCINTOVF.bit.ADCINT1)
    {
        AdcaRegs.ADCINTOVFCLR.bit.ADCINT1 = 1; //clear INT1 overflow flag
        AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // Acknowledge the interrupt
}
