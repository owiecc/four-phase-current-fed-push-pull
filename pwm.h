
#ifndef PWM_H_
#define PWM_H_

// Defines
#define PWM_PRD         3000 // Switching period fCPU/fSW = 120M/40k = 3000
#define PWM_PRD_HALF    1500 //
#define PWM_PRD_QUARTER  750 //

void updateModulator(float, float);
void initEPWM(void);
void updateEPWM(unsigned int, unsigned int, int);
void disablePWM(void);
void enablePWM(void);

#endif /* PWM_H_ */
