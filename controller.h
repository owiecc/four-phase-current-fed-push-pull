
#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "adc.h"

enum converter_states {StateInitDSP,StateStandby,StateStartup,StateOn,StateShutdown,StateFail};

struct SOALimits {
    float maxTrip;
    float maxStartup;
    float minStartup;
    float minTrip;
};

struct SOALimitsConverter {
    struct SOALimits Vin;
    struct SOALimits Vout;
    struct SOALimits Vclamp;
    struct SOALimits Iout;
};

int isInSOATrip(float, struct SOALimits);
int isInSOAStartup(float, struct SOALimits);
int isInSOA(struct ADCResult, enum converter_states);
__interrupt void adcA1ISR(void);

#endif /* CONTROLLER_H_ */
