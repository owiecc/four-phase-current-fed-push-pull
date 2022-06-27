
#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "adc.h"

enum converter_states {StateInitDSP,StateStandby,StateStartup,StateOn,StateShutdown,StateTrip};
enum trip_status {TripOC, TripSOAVin, TripSOAVout, TripSOAVclamp, NoTrip};

struct Range {
    float lo;
    float hi;
};

struct SOAConverter {
    struct Range Vin;
    struct Range Vout;
    struct Range Vclamp;
    struct Range Iout;
};

inline int inRange(float, struct Range);

enum trip_status isInSOA(struct ADCResult);

void initTripFeedback(enum trip_status *);

void initPIConttrollers(void);

void setControllerVclampRef(float);
void adjControllerVclampRef(float);

void setControllerIoutRef(float);
void adjControllerIoutRef(float);

__interrupt void adcA1ISR(void);

#endif /* CONTROLLER_H_ */
