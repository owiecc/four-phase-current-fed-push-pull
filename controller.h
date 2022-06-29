
#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "adc.h"

enum converter_states {StateInitDSP,StateStandby,StateStartup,StateOn,StateShutdown,StateTrip};
enum trip_status {TripOC, TripSOAVin, TripSOAVout, TripSOAVclamp, TripCycleLimit, NoTrip};

struct Range {
    float lo;
    float hi;
};

struct OPConverter {
    struct Range Vin;
    struct Range Vout;
    struct Range Vclamp;
    struct Range Iout;
};

extern struct OPConverter SOA; // safe operating area

inline int inRange(float, struct Range);
enum trip_status inRangeOP(struct ADCResult, struct OPConverter);
enum trip_status inSOA(struct ADCResult);

void initTripFeedback(enum trip_status *);

void initPIConttrollers(void);

void setControllerDeltaVclampRef(float);
void adjControllerDeltaVclampRef(float);

void setControllerIoutRef(float);
void adjControllerIoutRef(float);

__interrupt void adcA1ISR(void);

#endif /* CONTROLLER_H_ */
