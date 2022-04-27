
#ifndef PWM_H_
#define PWM_H_

// Defines
#define PWM_PRD         2500 // Switching period ~40kHz
#define PWM_PRD_HALF    1250 //
#define PWM_PRD_QUARTER  625 //

void updateModulator(float, float);
void initEPWM(void);
void updateEPWM(unsigned int, unsigned int, int);

#endif /* PWM_H_ */
