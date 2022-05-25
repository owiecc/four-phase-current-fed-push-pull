
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
    EPwm1Regs.TBPRD = PWM_PRD_HALF; // Set period to ~40kHz
    EPwm1Regs.CMPA.bit.CMPA = 0;        // Set duty cycle to 100% on high-side transistor
    EPwm1Regs.TBCTL.bit.CTRMODE = 2;    // Up/down mode
    EPwm1Regs.TBCTL.bit.PHSEN = 0;      // Master module
    EPwm1Regs.TBCTL.bit.PRDLD = 0;      // Shadow register
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0;  // TBCLK = SYSCLK
    EPwm1Regs.TBCTL.bit.CLKDIV = 0;
    EPwm1Regs.TBCTR = 0;
    EPwm1Regs.EPWMSYNCOUTEN.bit.ZEROEN = 1; // Sync out on CTR = 0
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = 0;
    EPwm1Regs.CMPCTL.bit.LOADAMODE = 0; // load on CTR = Zero
    EPwm1Regs.AQCTLA.bit.CAU = 2;       // High on counter up
    EPwm1Regs.AQCTLA.bit.CAD = 1;       // Low on counter down
    EPwm1Regs.DBCTL.bit.IN_MODE = 0;    // Channel A input
    EPwm1Regs.DBCTL.bit.DEDB_MODE = 0;  //
    EPwm1Regs.DBCTL.bit.POLSEL = 2;     // Active high, complementary
    EPwm1Regs.DBCTL.bit.OUT_MODE = 3;   // Channel A controls channel B
    EPwm1Regs.DBFED.bit.DBFED = 20;     // 200ns
    EPwm1Regs.DBRED.bit.DBRED = 20;     // 200ns
    EPwm1Regs.TZCTL.bit.TZA = TZ_FORCE_HI; // EPWM1A high on trip
    EPwm1Regs.TZCTL.bit.TZB = TZ_FORCE_LO; // EPWM1B low on trip

    EPwm1Regs.ETSEL.bit.SOCAEN = 1;     // Disable SOC on A group
    EPwm1Regs.ETSEL.bit.SOCASEL = 1;    // Select SOC on up-count
    EPwm1Regs.ETPS.bit.SOCAPRD = 1;     // Generate pulse on 1st event

    // PWM2 configuration
    EPwm2Regs.TBPRD = PWM_PRD_HALF;     // Set period to ~40kHz
    EPwm2Regs.CMPA.bit.CMPA = 0;        // Set duty cycle to 100% on high-side transistor
    EPwm2Regs.TBCTL.bit.CTRMODE = 2;    // Up/down mode
    EPwm2Regs.TBCTL.bit.PHSEN = 1;      // Slave module
    EPwm2Regs.TBCTL.bit.PRDLD = 0;      // Shadow register
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0;  // TBCLK = SYSCLK
    EPwm2Regs.TBCTL.bit.CLKDIV = 0;
    EPwm2Regs.TBCTR = 0;
    EPwm2Regs.TBPHS.bit.TBPHS = PWM_PRD_QUARTER; // π/2 phase delay
    EPwm2Regs.EPWMSYNCINSEL.bit.SEL = 1; // Sync to PWM1
    EPwm2Regs.EPWMSYNCOUTEN.bit.ZEROEN = 1; // Sync out on CTR = 0
    EPwm2Regs.CMPCTL.bit.SHDWAMODE = 0;
    EPwm2Regs.CMPCTL.bit.LOADAMODE = 0; // load on CTR = Zero
    EPwm2Regs.AQCTLA.bit.CAU = 2;       // High on counter up
    EPwm2Regs.AQCTLA.bit.CAD = 1;       // Low on counter down
    EPwm2Regs.DBCTL.bit.IN_MODE = 0;    // Channel A input
    EPwm2Regs.DBCTL.bit.DEDB_MODE = 0;  //
    EPwm2Regs.DBCTL.bit.POLSEL = 2;     // Active high, complementary
    EPwm2Regs.DBCTL.bit.OUT_MODE = 3;   // Channel A controls channel B
    EPwm2Regs.DBFED.bit.DBFED = 20;     // 200ns
    EPwm2Regs.DBRED.bit.DBRED = 20;     // 200ns
    EPwm2Regs.TZCTL.bit.TZA = TZ_FORCE_HI; // EPWM2A high on trip
    EPwm2Regs.TZCTL.bit.TZB = TZ_FORCE_LO; // EPWM2B low on trip

    // PWM3 configuration
    EPwm3Regs.TBPRD = PWM_PRD_HALF;     // Set period to ~40kHz
    EPwm3Regs.CMPA.bit.CMPA = 0;        // Set duty cycle to 100% on high-side transistor
    EPwm3Regs.TBCTL.bit.CTRMODE = 2;    // Up/down mode
    EPwm3Regs.TBCTL.bit.PHSEN = 1;      // Slave module
    EPwm3Regs.TBCTL.bit.PRDLD = 0;      // Shadow register
    EPwm3Regs.TBCTL.bit.HSPCLKDIV = 0;  // TBCLK = SYSCLK
    EPwm3Regs.TBCTL.bit.CLKDIV = 0;
    EPwm3Regs.TBCTR = 0;
    EPwm3Regs.TBPHS.bit.TBPHS = 0;      // zero phase delay
    EPwm3Regs.EPWMSYNCINSEL.bit.SEL = 1; // Sync to PWM1
    EPwm3Regs.EPWMSYNCOUTEN.bit.ZEROEN = 1; // Sync out on CTR = 0
    EPwm3Regs.CMPCTL.bit.SHDWAMODE = 0;
    EPwm3Regs.CMPCTL.bit.LOADAMODE = 0; // load on CTR = Zero
    EPwm3Regs.AQCTLA.bit.CAU = 2;       // High on counter up
    EPwm3Regs.AQCTLA.bit.CAD = 1;       // Low on counter down
    EPwm3Regs.DBCTL.bit.IN_MODE = 0;    // Channel A input
    EPwm3Regs.DBCTL.bit.DEDB_MODE = 0;  //
    EPwm3Regs.DBCTL.bit.POLSEL = 2;     // Active high, complementary
    EPwm3Regs.DBCTL.bit.OUT_MODE = 3;   // Channel A controls channel B
    EPwm3Regs.DBFED.bit.DBFED = 20;     // 200ns
    EPwm3Regs.DBRED.bit.DBRED = 20;     // 200ns
    EPwm3Regs.TZCTL.bit.TZA = TZ_FORCE_HI; // EPWM3A high on trip
    EPwm3Regs.TZCTL.bit.TZB = TZ_FORCE_LO; // EPWM3B low on trip

    // PWM4 configuration
    EPwm4Regs.TBPRD = PWM_PRD_HALF;     // Set period to ~40kHz
    EPwm4Regs.CMPA.bit.CMPA = 0;        // Set duty cycle to 100% on high-side transistor
    EPwm4Regs.TBCTL.bit.CTRMODE = 2;    // Up/down mode
    EPwm4Regs.TBCTL.bit.PHSEN = 1;      // Slave module
    EPwm4Regs.TBCTL.bit.PRDLD = 0;      // Shadow register
    EPwm4Regs.TBCTL.bit.HSPCLKDIV = 0;  // TBCLK = SYSCLK
    EPwm4Regs.TBCTL.bit.CLKDIV = 0;
    EPwm4Regs.TBCTR = 0;
    EPwm4Regs.TBPHS.bit.TBPHS = PWM_PRD_QUARTER; // π/2 phase delay
    EPwm4Regs.EPWMSYNCINSEL.bit.SEL = 3; // Sync to PWM3
    EPwm4Regs.EPWMSYNCOUTEN.bit.ZEROEN = 1; // Sync out on CTR = 0
    EPwm4Regs.CMPCTL.bit.SHDWAMODE = 0;
    EPwm4Regs.CMPCTL.bit.LOADAMODE = 0; // load on CTR = Zero
    EPwm4Regs.AQCTLA.bit.CAU = 2;       // High on counter up
    EPwm4Regs.AQCTLA.bit.CAD = 1;       // Low on counter down
    EPwm4Regs.DBCTL.bit.IN_MODE = 0;    // Channel A input
    EPwm4Regs.DBCTL.bit.DEDB_MODE = 0;  //
    EPwm4Regs.DBCTL.bit.POLSEL = 2;     // Active high, complementary
    EPwm4Regs.DBCTL.bit.OUT_MODE = 3;   // Channel A controls channel B
    EPwm4Regs.DBFED.bit.DBFED = 20;     // 200ns
    EPwm4Regs.DBRED.bit.DBRED = 20;     // 200ns
    EPwm4Regs.TZCTL.bit.TZA = TZ_FORCE_HI; // EPWM4A high on trip
    EPwm4Regs.TZCTL.bit.TZB = TZ_FORCE_LO; // EPWM4B low on trip

    // PWM5 configuration
    EPwm5Regs.TBPRD = PWM_PRD_HALF;     // Set period to ~40kHz
    EPwm5Regs.CMPA.bit.CMPA = 0;        // Set duty cycle to 100% on high-side transistor
    EPwm5Regs.TBCTL.bit.CTRMODE = 2;    // Up/down mode
    EPwm5Regs.TBCTL.bit.PHSEN = 1;      // Slave module
    EPwm5Regs.TBCTL.bit.PRDLD = 0;      // Shadow register
    EPwm5Regs.TBCTL.bit.HSPCLKDIV = 0;  // TBCLK = SYSCLK
    EPwm5Regs.TBCTL.bit.CLKDIV = 0;
    EPwm5Regs.TBCTR = 0;
    EPwm5Regs.TBPHS.bit.TBPHS = PWM_PRD_QUARTER ; // π/2 phase delay
    EPwm5Regs.EPWMSYNCINSEL.bit.SEL = 2; // Sync to PWM2
    EPwm5Regs.EPWMSYNCOUTEN.bit.ZEROEN = 1; // Sync out on CTR = 0
    EPwm5Regs.CMPCTL.bit.SHDWAMODE = 0;
    EPwm5Regs.CMPCTL.bit.LOADAMODE = 0; // load on CTR = Zero
    EPwm5Regs.AQCTLA.bit.CAU = 2;       // High on counter up
    EPwm5Regs.AQCTLA.bit.CAD = 1;       // Low on counter down
    EPwm5Regs.DBCTL.bit.IN_MODE = 0;    // Channel A input
    EPwm5Regs.DBCTL.bit.DEDB_MODE = 0;  //
    EPwm5Regs.DBCTL.bit.POLSEL = 2;     // Active high, complementary
    EPwm5Regs.DBCTL.bit.OUT_MODE = 3;   // Channel A controls channel B
    EPwm5Regs.DBFED.bit.DBFED = 20;     // 200ns
    EPwm5Regs.DBRED.bit.DBRED = 20;     // 200ns
    EPwm5Regs.TZCTL.bit.TZA = TZ_FORCE_HI; // EPWM5A high on trip
    EPwm5Regs.TZCTL.bit.TZB = TZ_FORCE_LO; // EPWM5B low on trip

    // PWM6 configuration
    EPwm6Regs.TBPRD = PWM_PRD_HALF;     // Set period to ~40kHz
    EPwm6Regs.CMPA.bit.CMPA = 0;        // Set duty cycle to 100% on high-side transistor
    EPwm6Regs.TBCTL.bit.CTRMODE = 2;    // Up/down mode
    EPwm6Regs.TBCTL.bit.PHSEN = 1;      // Slave module
    EPwm6Regs.TBCTL.bit.PRDLD = 0;      // Shadow register
    EPwm6Regs.TBCTL.bit.HSPCLKDIV = 0;  // TBCLK = SYSCLK
    EPwm6Regs.TBCTL.bit.CLKDIV = 0;
    EPwm6Regs.TBCTR = 0;
    EPwm6Regs.TBPHS.bit.TBPHS = PWM_PRD_QUARTER; // π/2 phase delay
    EPwm6Regs.EPWMSYNCINSEL.bit.SEL = 5; // Sync to PWM5
    EPwm6Regs.EPWMSYNCOUTEN.bit.ZEROEN = 1; // Sync out on CTR = 0
    EPwm6Regs.CMPCTL.bit.SHDWAMODE = 0;
    EPwm6Regs.CMPCTL.bit.LOADAMODE = 0; // load on CTR = Zero
    EPwm6Regs.AQCTLA.bit.CAU = 2;       // High on counter up
    EPwm6Regs.AQCTLA.bit.CAD = 1;       // Low on counter down
    EPwm6Regs.DBCTL.bit.IN_MODE = 0;    // Channel A input
    EPwm6Regs.DBCTL.bit.DEDB_MODE = 0;  //
    EPwm6Regs.DBCTL.bit.POLSEL = 2;     // Active high, complementary
    EPwm6Regs.DBCTL.bit.OUT_MODE = 3;   // Channel A controls channel B
    EPwm6Regs.DBFED.bit.DBFED = 20;     // 200ns
    EPwm6Regs.DBRED.bit.DBRED = 20;     // 200ns
    EPwm6Regs.TZCTL.bit.TZA = TZ_FORCE_HI; // EPWM6A high on trip
    EPwm6Regs.TZCTL.bit.TZB = TZ_FORCE_LO; // EPWM6B low on trip

    // PWM7 configuration
    EPwm7Regs.TBPRD = PWM_PRD_HALF;     // Set period to ~40kHz
    EPwm7Regs.CMPA.bit.CMPA = 0;        // Set duty cycle to 100% on high-side transistor
    EPwm7Regs.TBCTL.bit.CTRMODE = 2;    // Up/down mode
    EPwm7Regs.TBCTL.bit.PHSEN = 1;      // Slave module
    EPwm7Regs.TBCTL.bit.PRDLD = 0;      // Shadow register
    EPwm7Regs.TBCTL.bit.HSPCLKDIV = 0;  // TBCLK = SYSCLK
    EPwm7Regs.TBCTL.bit.CLKDIV = 0;
    EPwm7Regs.TBCTR = 0;
    EPwm7Regs.TBPHS.bit.TBPHS = PWM_PRD_QUARTER; // π/2 phase delay
    EPwm7Regs.EPWMSYNCINSEL.bit.SEL = 4; // Sync to PWM4
    EPwm7Regs.EPWMSYNCOUTEN.bit.ZEROEN = 1; // Sync out on CTR = 0
    EPwm7Regs.CMPCTL.bit.SHDWAMODE = 0;
    EPwm7Regs.CMPCTL.bit.LOADAMODE = 0; // load on CTR = Zero
    EPwm7Regs.AQCTLA.bit.CAU = 2;       // High on counter up
    EPwm7Regs.AQCTLA.bit.CAD = 1;       // Low on counter down
    EPwm7Regs.DBCTL.bit.IN_MODE = 0;    // Channel A input
    EPwm7Regs.DBCTL.bit.DEDB_MODE = 0;  //
    EPwm7Regs.DBCTL.bit.POLSEL = 2;     // Active high, complementary
    EPwm7Regs.DBCTL.bit.OUT_MODE = 3;   // Channel A controls channel B
    EPwm7Regs.DBFED.bit.DBFED = 20;     // 200ns
    EPwm7Regs.DBRED.bit.DBRED = 20;     // 200ns
    EPwm7Regs.TZCTL.bit.TZA = TZ_FORCE_HI; // EPWM7A high on trip
    EPwm7Regs.TZCTL.bit.TZB = TZ_FORCE_LO; // EPWM7B low on trip

    // PWM8 configuration
    EPwm8Regs.TBPRD = PWM_PRD_HALF;     // Set period to ~40kHz
    EPwm8Regs.CMPA.bit.CMPA = 0;        // Set duty cycle to 100% on high-side transistor
    EPwm8Regs.TBCTL.bit.CTRMODE = 2;    // Up/down mode
    EPwm8Regs.TBCTL.bit.PHSEN = 1;      // Slave module
    EPwm8Regs.TBCTL.bit.PRDLD = 0;      // Shadow register
    EPwm8Regs.TBCTL.bit.HSPCLKDIV = 0;  // TBCLK = SYSCLK
    EPwm8Regs.TBCTL.bit.CLKDIV = 0;
    EPwm8Regs.TBCTR = 0;
    EPwm8Regs.TBPHS.bit.TBPHS = PWM_PRD_QUARTER; // π/2 phase delay
    EPwm8Regs.EPWMSYNCINSEL.bit.SEL = 7; // Sync to PWM7
    EPwm8Regs.CMPCTL.bit.SHDWAMODE = 0;
    EPwm8Regs.CMPCTL.bit.LOADAMODE = 0; // load on CTR = Zero
    EPwm8Regs.AQCTLA.bit.CAU = 2;       // High on counter up
    EPwm8Regs.AQCTLA.bit.CAD = 1;       // Low on counter down
    EPwm8Regs.DBCTL.bit.IN_MODE = 0;    // Channel A input
    EPwm8Regs.DBCTL.bit.DEDB_MODE = 0;  //
    EPwm8Regs.DBCTL.bit.POLSEL = 2;     // Active high, complementary
    EPwm8Regs.DBCTL.bit.OUT_MODE = 3;   // Channel A controls channel B
    EPwm8Regs.DBFED.bit.DBFED = 20;     // 200ns
    EPwm8Regs.DBRED.bit.DBRED = 20;     // 200ns
    EPwm8Regs.TZCTL.bit.TZA = TZ_FORCE_HI; // EPWM8A high on trip
    EPwm8Regs.TZCTL.bit.TZB = TZ_FORCE_LO; // EPWM8B low on trip

    EDIS;
}

void disablePWM(void) {
    EALLOW;
    EPwm1Regs.TZFRC.bit.OST = 0x1;
    EPwm2Regs.TZFRC.bit.OST = 0x1;
    EPwm3Regs.TZFRC.bit.OST = 0x1;
    EPwm4Regs.TZFRC.bit.OST = 0x1;
    EPwm5Regs.TZFRC.bit.OST = 0x1;
    EPwm6Regs.TZFRC.bit.OST = 0x1;
    EPwm7Regs.TZFRC.bit.OST = 0x1;
    EPwm8Regs.TZFRC.bit.OST = 0x1;
    EDIS;
}

void enablePWM(void) {
    EALLOW;
    EPwm1Regs.TZCLR.bit.OST = 0x1;
    EPwm2Regs.TZCLR.bit.OST = 0x1;
    EPwm3Regs.TZCLR.bit.OST = 0x1;
    EPwm4Regs.TZCLR.bit.OST = 0x1;
    EPwm5Regs.TZCLR.bit.OST = 0x1;
    EPwm6Regs.TZCLR.bit.OST = 0x1;
    EPwm7Regs.TZCLR.bit.OST = 0x1;
    EPwm8Regs.TZCLR.bit.OST = 0x1;
    EDIS;
}

