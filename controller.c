
#include "PI_controller.h"
#include "controller.h"
#include "f28x_project.h"
#include "pwm.h"
#include "leds.h"

#define n1 14
#define n2 21
#define N ((float)n1/(float)n2)
#define Ninv ((float)n2/(float)n1)
#define FSW 40000
#define PI_Vc_Ki 0.05f
#define PI_Io_Ki 0.10f
#define CYCLE_LIMIT 60000

static struct piController PI_Vc = {0, 0, 0, 0, 0}; // Vclamp controller
static struct piController PI_Io = {0, 0, 0, 0, 0}; // Iout controller

static float refIo = 0.0f; // value overwritten in main.c
static float refDeltaVclamp = 0.0f;

static enum trip_status * tripFeedback;

struct OPConverter SOA = {
    .Vin =    (struct Range) {.lo = 720.0f, .hi =  840.0f},
    .Vout =   (struct Range) {.lo = 190.0f, .hi =  925.0f},
    .Vclamp = (struct Range) {.lo = -10.0f, .hi = 1260.0f},
    .Iout =   (struct Range) {.lo =  -6.0f, .hi =    6.0f}
};

inline int inRange(float x, struct Range r)
{
    return (x<r.hi && x>r.lo) ? 1 : 0;
}

enum trip_status inRangeOP(struct ADCResult sensors, struct OPConverter op)
{
    enum trip_status is_tripped = NoTrip;
    is_tripped = inRange(sensors.Iout, op.Iout) ? is_tripped : TripOC;
    is_tripped = inRange(sensors.Vclamp, op.Vclamp) ? is_tripped : TripSOAVclamp;
    is_tripped = inRange(sensors.Vout, op.Vout) ? is_tripped : TripSOAVout;
    is_tripped = inRange(sensors.Vin, op.Vin) ? is_tripped : TripSOAVin;
    return is_tripped;
}

enum trip_status inSOA(struct ADCResult sensors)
{
    return inRangeOP(sensors, SOA);
}

void initTripFeedback(enum trip_status *x)
{
    tripFeedback = x;
}

void initPIConttrollers(void)
{
    PI_Vc = initPI(PI_Vc_Ki/FSW, 2*PI_Vc_Ki/FSW, 0.5, -1, 0.99, 1.0, 0.2);
    PI_Io = initPI(PI_Io_Ki/FSW, 2*PI_Io_Ki/FSW, 0.5, -1, 0.00, 0.25, -0.25);
}

void setControllerDeltaVclampRef(float x) { refDeltaVclamp = x; }
void adjControllerDeltaVclampRef(float x) { refDeltaVclamp += x; }

void setControllerIoutRef(float x) { refIo = x; }
void adjControllerIoutRef(float x) { refIo += x; }

// adcA1ISR - ADC A Interrupt 1 ISR
// Runs with every switching cycle, runs the control law for the converter
__interrupt void adcA1ISR(void)
{
    struct ADCResult meas = scaleADCs();

    static unsigned int ncycles = CYCLE_LIMIT;

    if (ncycles-- == 0) // trip
    {
        disablePWM();
        *tripFeedback = TripOC;//isInSOA(meas, StateOn);
        ncycles = CYCLE_LIMIT;
    }
    else // normal operation
    {
        float errVclamp = meas.Vin*Ninv + refDeltaVclamp - meas.Vclamp;
        float errIout = refIo - meas.Iout;

        float d = updatePI(&PI_Vc, -errVclamp);
        float p = updatePI(&PI_Io, errIout);

        updateModulator(d, p);

        if (abs(errVclamp) < 10.0f) { ledOn(LEDOKVclampRegulator); } else { ledOff(LEDOKVclampRegulator); }
        if (abs(errIout) < 0.1f) { ledOn(LEDOKIoRegulator); } else { ledOff(LEDOKIoRegulator); }
    }

    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // Clear the interrupt flag

    // Check if overflow has occurred
    if(1 == AdcaRegs.ADCINTOVF.bit.ADCINT1)
    {
        AdcaRegs.ADCINTOVFCLR.bit.ADCINT1 = 1; //clear INT1 overflow flag
        AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // Acknowledge the interrupt
}
