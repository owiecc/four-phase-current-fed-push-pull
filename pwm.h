
#ifndef PWM_H_
#define PWM_H_

// Defines
#define PWM_PRD_HALF     625 // Quarter of a switching period ~40kHz

void initEPWM(unsigned int);
void updateEPWM(unsigned int, unsigned int, int);

#endif /* PWM_H_ */
