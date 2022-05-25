
#include "pwm.h"
#include "f28x_project.h"

// # Structure of the PWM modulator
//
// - EPwm1 + EPwm2 + EPwm5 + EPwm6 = grid side bridge
// - EPwm3 + EPwm4 + EPwm7 + EPwm8 = battery side bridge
//
// ## Phase delays between individual leg modulators
//
// EPwm1 --- (+π/2) EPwm2 --- (+π/2) EPwm5 --- (+π/2) EPwm6
//   |
//   └-- (+Θ) EPwm3 --- (+π/2) EPwm4 --- (+π/2) EPwm7 --- (+π/2) EPwm8
//

void updateModulator(float d, float phase)
{
    Uint16 cmp = PWM_PRD_HALF*(1-d);
    int ph = -PWM_PRD*phase;
    updateEPWM(cmp, cmp, ph);
}

// initEPWM - Function to configure ePWM1 to generate the SOC.
void updateEPWM(unsigned int cmpIn, unsigned int cmpOut, int phaseShiftInOut)
{
    // Set compare A values for input bridge
    EPwm1Regs.CMPA.bit.CMPA = cmpIn;
    EPwm2Regs.CMPA.bit.CMPA = cmpIn;
    EPwm5Regs.CMPA.bit.CMPA = cmpIn;
    EPwm6Regs.CMPA.bit.CMPA = cmpIn;

    // Set compare A values for output bridge
    EPwm3Regs.CMPA.bit.CMPA = cmpOut;
    EPwm4Regs.CMPA.bit.CMPA = cmpOut;
    EPwm7Regs.CMPA.bit.CMPA = cmpOut;
    EPwm8Regs.CMPA.bit.CMPA = cmpOut;

    // Set phase delay for output bridge
    EPwm3Regs.TBCTL.bit.PHSDIR = phaseShiftInOut > 0 ? 1 : 0; // Count up on positive shift and down on negative
    EPwm3Regs.TBPHS.bit.TBPHS = abs(phaseShiftInOut); // Θ phase delay w.r.t. PWM1
}

// initEPWM - Function to configure ePWM1 to generate the SOC.
void initEPWM(void)
{
    EALLOW;
    // PWM1 configuration
    EPwm1Regs.TBPRD = PWM_PRD_HALF;                 // Set period to 40kHz
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;         // Master module
    EPwm1Regs.TBCTL.bit.PRDLD = TB_SHADOW;          // Shadow register
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0;              // TBCLK = SYSCLK
    EPwm1Regs.EPWMSYNCOUTEN.bit.ZEROEN = 1;         // Sync out on CTR = 0
    EPwm1Regs.AQCTLA.bit.CAU = AQ_SET;              // High on counter up
    EPwm1Regs.AQCTLA.bit.CAD = AQ_CLEAR;            // Low on counter down         //
    EPwm1Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;       // Active high, complementary
    EPwm1Regs.DBCTL.bit.OUT_MODE = 3;               // Channel A controls channel B
    EPwm1Regs.DBFED.bit.DBFED = 20;                 // 200ns
    EPwm1Regs.DBRED.bit.DBRED = 20;                 // 200ns
    EPwm1Regs.TZCTL.bit.TZA = TZ_FORCE_HI;          // EPWM1A high on trip
    EPwm1Regs.TZCTL.bit.TZB = TZ_FORCE_LO;          // EPWM1B low on trip

    EPwm1Regs.ETSEL.bit.SOCAEN = 1;                 // Enable SOC on A group
    EPwm1Regs.ETSEL.bit.SOCASEL = 1;                // Select SOC on up-count
    EPwm1Regs.ETPS.bit.SOCAPRD = 1;                 // Generate pulse on 1st event

    // PWM2 configuration
    EPwm2Regs.TBPRD = PWM_PRD_HALF;
    EPwm2Regs.TBCTL.bit.PHSEN = TB_ENABLE;
    EPwm2Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0;
    EPwm2Regs.TBPHS.bit.TBPHS = PWM_PRD_QUARTER;    // π/2 phase delay
    EPwm2Regs.EPWMSYNCINSEL.bit.SEL = 1;            // Sync to PWM1
    EPwm2Regs.EPWMSYNCOUTEN.bit.ZEROEN = 1;
    EPwm2Regs.AQCTLA.bit.CAU = AQ_SET;
    EPwm2Regs.AQCTLA.bit.CAD = AQ_CLEAR;
    EPwm2Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;
    EPwm2Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm2Regs.DBFED.bit.DBFED = 20;
    EPwm2Regs.DBRED.bit.DBRED = 20;
    EPwm2Regs.TZCTL.bit.TZA = TZ_FORCE_HI;
    EPwm2Regs.TZCTL.bit.TZB = TZ_FORCE_LO;

    // PWM3 configuration
    EPwm3Regs.TBPRD = PWM_PRD_HALF;
    EPwm3Regs.TBCTL.bit.PHSEN = TB_ENABLE;
    EPwm3Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    EPwm3Regs.TBCTL.bit.HSPCLKDIV = 0;
    EPwm3Regs.TBPHS.bit.TBPHS = 0;                  // zero phase delay
    EPwm3Regs.EPWMSYNCINSEL.bit.SEL = 1;            // Sync to PWM1
    EPwm3Regs.EPWMSYNCOUTEN.bit.ZEROEN = 1;
    EPwm3Regs.AQCTLA.bit.CAU = AQ_SET;
    EPwm3Regs.AQCTLA.bit.CAD = AQ_CLEAR;
    EPwm3Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;
    EPwm3Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm3Regs.DBFED.bit.DBFED = 20;
    EPwm3Regs.DBRED.bit.DBRED = 20;
    EPwm3Regs.TZCTL.bit.TZA = TZ_FORCE_HI;
    EPwm3Regs.TZCTL.bit.TZB = TZ_FORCE_LO;

    // PWM4 configuration
    EPwm4Regs.TBPRD = PWM_PRD_HALF;
    EPwm4Regs.TBCTL.bit.PHSEN = TB_ENABLE;
    EPwm4Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    EPwm4Regs.TBCTL.bit.HSPCLKDIV = 0;
    EPwm4Regs.TBPHS.bit.TBPHS = PWM_PRD_QUARTER;    // π/2 phase delay
    EPwm4Regs.EPWMSYNCINSEL.bit.SEL = 3;            // Sync to PWM3
    EPwm4Regs.EPWMSYNCOUTEN.bit.ZEROEN = 1;
    EPwm4Regs.AQCTLA.bit.CAU = AQ_SET;
    EPwm4Regs.AQCTLA.bit.CAD = AQ_CLEAR;
    EPwm4Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;
    EPwm4Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm4Regs.DBFED.bit.DBFED = 20;
    EPwm4Regs.DBRED.bit.DBRED = 20;
    EPwm4Regs.TZCTL.bit.TZA = TZ_FORCE_HI;
    EPwm4Regs.TZCTL.bit.TZB = TZ_FORCE_LO;

    // PWM5 configuration
    EPwm5Regs.TBPRD = PWM_PRD_HALF;
    EPwm5Regs.TBCTL.bit.PHSEN = TB_ENABLE;
    EPwm5Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    EPwm5Regs.TBCTL.bit.HSPCLKDIV = 0;
    EPwm5Regs.TBPHS.bit.TBPHS = PWM_PRD_QUARTER ;   // π/2 phase delay
    EPwm5Regs.EPWMSYNCINSEL.bit.SEL = 2;            // Sync to PWM2
    EPwm5Regs.EPWMSYNCOUTEN.bit.ZEROEN = 1;
    EPwm5Regs.AQCTLA.bit.CAU = AQ_SET;
    EPwm5Regs.AQCTLA.bit.CAD = AQ_CLEAR;
    EPwm5Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;
    EPwm5Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm5Regs.DBFED.bit.DBFED = 20;
    EPwm5Regs.DBRED.bit.DBRED = 20;
    EPwm5Regs.TZCTL.bit.TZA = TZ_FORCE_HI;
    EPwm5Regs.TZCTL.bit.TZB = TZ_FORCE_LO;

    // PWM6 configuration
    EPwm6Regs.TBPRD = PWM_PRD_HALF;
    EPwm6Regs.TBCTL.bit.PHSEN = TB_ENABLE;
    EPwm6Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    EPwm6Regs.TBCTL.bit.HSPCLKDIV = 0;
    EPwm6Regs.TBPHS.bit.TBPHS = PWM_PRD_QUARTER;    // π/2 phase delay
    EPwm6Regs.EPWMSYNCINSEL.bit.SEL = 5;            // Sync to PWM5
    EPwm6Regs.EPWMSYNCOUTEN.bit.ZEROEN = 1;
    EPwm6Regs.AQCTLA.bit.CAU = AQ_SET;
    EPwm6Regs.AQCTLA.bit.CAD = AQ_CLEAR;
    EPwm6Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;
    EPwm6Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm6Regs.DBFED.bit.DBFED = 20;
    EPwm6Regs.DBRED.bit.DBRED = 20;
    EPwm6Regs.TZCTL.bit.TZA = TZ_FORCE_HI;
    EPwm6Regs.TZCTL.bit.TZB = TZ_FORCE_LO;

    // PWM7 configuration
    EPwm7Regs.TBPRD = PWM_PRD_HALF;
    EPwm7Regs.TBCTL.bit.PHSEN = TB_ENABLE;
    EPwm7Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    EPwm7Regs.TBCTL.bit.HSPCLKDIV = 0;
    EPwm7Regs.TBPHS.bit.TBPHS = PWM_PRD_QUARTER;    // π/2 phase delay
    EPwm7Regs.EPWMSYNCINSEL.bit.SEL = 4;            // Sync to PWM4
    EPwm7Regs.EPWMSYNCOUTEN.bit.ZEROEN = 1;
    EPwm7Regs.AQCTLA.bit.CAU = AQ_SET;
    EPwm7Regs.AQCTLA.bit.CAD = AQ_CLEAR;
    EPwm7Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;
    EPwm7Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm7Regs.DBFED.bit.DBFED = 20;
    EPwm7Regs.DBRED.bit.DBRED = 20;
    EPwm7Regs.TZCTL.bit.TZA = TZ_FORCE_HI;
    EPwm7Regs.TZCTL.bit.TZB = TZ_FORCE_LO;

    // PWM8 configuration
    EPwm8Regs.TBPRD = PWM_PRD_HALF;
    EPwm8Regs.TBCTL.bit.PHSEN = TB_ENABLE;
    EPwm8Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    EPwm8Regs.TBCTL.bit.HSPCLKDIV = 0;
    EPwm8Regs.TBPHS.bit.TBPHS = PWM_PRD_QUARTER;    // π/2 phase delay
    EPwm8Regs.EPWMSYNCINSEL.bit.SEL = 7;            // Sync to PWM7
    EPwm8Regs.AQCTLA.bit.CAU = AQ_SET;
    EPwm8Regs.AQCTLA.bit.CAD = AQ_CLEAR;
    EPwm8Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;
    EPwm8Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm8Regs.DBFED.bit.DBFED = 20;
    EPwm8Regs.DBRED.bit.DBRED = 20;
    EPwm8Regs.TZCTL.bit.TZA = TZ_FORCE_HI;
    EPwm8Regs.TZCTL.bit.TZB = TZ_FORCE_LO;
    EDIS;

    disablePWM();
}

void disablePWM(void) {
    EALLOW;
    // Force trip
    EPwm1Regs.TZFRC.bit.OST = 1;
    EPwm2Regs.TZFRC.bit.OST = 1;
    EPwm3Regs.TZFRC.bit.OST = 1;
    EPwm4Regs.TZFRC.bit.OST = 1;
    EPwm5Regs.TZFRC.bit.OST = 1;
    EPwm6Regs.TZFRC.bit.OST = 1;
    EPwm7Regs.TZFRC.bit.OST = 1;
    EPwm8Regs.TZFRC.bit.OST = 1;

    // Disable counters
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_FREEZE; 
    EPwm2Regs.TBCTL.bit.CTRMODE = TB_FREEZE; 
    EPwm3Regs.TBCTL.bit.CTRMODE = TB_FREEZE; 
    EPwm4Regs.TBCTL.bit.CTRMODE = TB_FREEZE; 
    EPwm5Regs.TBCTL.bit.CTRMODE = TB_FREEZE; 
    EPwm6Regs.TBCTL.bit.CTRMODE = TB_FREEZE; 
    EPwm7Regs.TBCTL.bit.CTRMODE = TB_FREEZE; 
    EPwm8Regs.TBCTL.bit.CTRMODE = TB_FREEZE; 
    EDIS;
}

void enablePWM(void) {
    EALLOW;
    // Set counter value
    EPwm1Regs.TBCTR = 0;
    EPwm2Regs.TBCTR = 0;
    EPwm3Regs.TBCTR = 0;
    EPwm4Regs.TBCTR = 0;
    EPwm5Regs.TBCTR = 0;
    EPwm6Regs.TBCTR = 0;
    EPwm7Regs.TBCTR = 0;
    EPwm8Regs.TBCTR = 0;

    // Set duty cycle to 100% on high-side transistor
    EPwm1Regs.CMPA.bit.CMPA = 0; 
    EPwm2Regs.CMPA.bit.CMPA = 0; 
    EPwm3Regs.CMPA.bit.CMPA = 0; 
    EPwm4Regs.CMPA.bit.CMPA = 0; 
    EPwm5Regs.CMPA.bit.CMPA = 0; 
    EPwm6Regs.CMPA.bit.CMPA = 0; 
    EPwm7Regs.CMPA.bit.CMPA = 0; 
    EPwm8Regs.CMPA.bit.CMPA = 0; 

    // Counter up/down mode
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; 
    EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; 
    EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; 
    EPwm4Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; 
    EPwm5Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; 
    EPwm6Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; 
    EPwm7Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; 
    EPwm8Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; 

    // Clear trip
    EPwm1Regs.TZCLR.bit.OST = 1;
    EPwm2Regs.TZCLR.bit.OST = 1;
    EPwm3Regs.TZCLR.bit.OST = 1;
    EPwm4Regs.TZCLR.bit.OST = 1;
    EPwm5Regs.TZCLR.bit.OST = 1;
    EPwm6Regs.TZCLR.bit.OST = 1;
    EPwm7Regs.TZCLR.bit.OST = 1;
    EPwm8Regs.TZCLR.bit.OST = 1;
    EDIS;
}
