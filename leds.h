
#ifndef LEDS_H_
#define LEDS_H_

enum leds {LEDTripOC, LEDTripSOAVin, LEDTripSOAVout, LEDTripSOAVclamp, LEDOKIoRegulator, LEDOKVclampRegulator};

void ledOn(enum leds);
void ledOff(enum leds);

#endif /* LEDS_H_ */
