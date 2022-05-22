
#ifndef PWM_H_
#define PWM_H_

// Defines
#define CPUFREQ 120000000
#define FSW 40000

#define PWM_PRD CPUFREQ/FSW // Switching period fCPU/fSW = 120M/40k = 3000
#define PWM_PRD_HALF CPUFREQ/FSW/2
#define PWM_PRD_QUARTER CPUFREQ/FSW/4

void updateModulator(float, float);
void initEPWM(void);
void updateEPWM(unsigned int, unsigned int, int);
void disablePWM(void);
void enablePWM(void);

#endif /* PWM_H_ */
