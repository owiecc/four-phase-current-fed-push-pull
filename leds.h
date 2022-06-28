
#ifndef LEDS_H_
#define LEDS_H_

enum leds {LEDTripOC, LEDTripSOAVin, LEDTripSOAVout, LEDTripSOAVclamp, LEDIoutAdjust, LEDVclampAdjust};

void ledOn(enum leds);
void ledOff(enum leds);
void ledsOff();

#endif /* LEDS_H_ */
