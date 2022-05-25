
#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "adc.h"

enum converter_states {StateInitDSP,StateStandby,StateStartup,StateOn,StateShutdown,StateTrip};
enum trip_status {TripOC, TripSOAVin, TripSOAVout, TripSOAVclamp, NoTrip};

struct OPLimits {
    float tripHi;    // trip above this value
    float startupHi; // startup allowed below this value
    float startupLo; // startup allowed above this value
    float tripLo;    // trip below this value
};

struct OPLimitsConverter {
    struct OPLimits Vin;
    struct OPLimits Vout;
    struct OPLimits Vclamp;
    struct OPLimits Iout;
};

int isInSOAOn(float, struct OPLimits);
int isInSOAStartup(float, struct OPLimits);
enum trip_status isInSOA(struct ADCResult, enum converter_states);

void initPIConttrollers(void);

void setControllerVclampRef(float);
void adjControllerVclampRef(float);

void setControllerIoutRef(float);
void adjControllerIoutRef(float);

__interrupt void adcA1ISR(void);

#endif /* CONTROLLER_H_ */
