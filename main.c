
#include "f28002x_device.h"
#include "f28x_project.h"

// Defines
#define PWM_PRD_HALF     625 // Quarter of a switching period ~40kHz

// Function prototypes
void initADC(void);
void initEPWM(void);
void updateEPWM(Uint16, Uint16, int16);
void initADCSOC(void);
__interrupt void adcA1ISR(void);

// Globals
Uint16 cmp1 = PWM_PRD_HALF;
Uint16 cmp2 = PWM_PRD_HALF;
int16 phs = 0;

// Main
void main(void)
{
    InitSysCtrl(); // Init device clock and peripherals
    InitGpio(); // Init GPIO

    EALLOW;
    // GPIO0-GPIO15 (PWM1-8, A+B)
    GpioCtrlRegs.GPAPUD.all &= 0xFFFF0000; // enable pullups
    GpioCtrlRegs.GPAMUX1.all = 0x55555555; // set PWM function

    // LEDs
    GpioCtrlRegs.GPAPUD.bit.GPIO22 = 0;  // LED Overvoltage: Vclamp
    GpioCtrlRegs.GPAMUX2.bit.GPIO22 = 0; // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO22 = 1;  // output

    GpioCtrlRegs.GPAPUD.bit.GPIO23 = 0;  // LED Overvoltage: Vout
    GpioCtrlRegs.GPAMUX2.bit.GPIO23 = 0; // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO23 = 1;  // output

    GpioCtrlRegs.GPAPUD.bit.GPIO28 = 0;  // LED Overvoltage: Vin
    GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 0; // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO28 = 1;  // output

    GpioCtrlRegs.GPAPUD.bit.GPIO29 = 0;  // LED Overcurrent
    GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 0; // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO29 = 1;  // output

    GpioCtrlRegs.GPAPUD.bit.GPIO30 = 0;  // LED PI regulator OK: Io
    GpioCtrlRegs.GPAMUX2.bit.GPIO30 = 0; // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO30 = 1;  // output

    GpioCtrlRegs.GPAPUD.bit.GPIO31 = 0;  // LED PI regulator OK: Vclamp
    GpioCtrlRegs.GPAMUX2.bit.GPIO31 = 0; // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO31 = 1;  // output

    // Button inputs
    GpioCtrlRegs.GPBCTRL.bit.QUALPRD0 = 1; // Qual period = SYSCLKOUT/2
    GpioCtrlRegs.GPBDIR.bit.GPIO39 = 0;    // input
    GpioCtrlRegs.GPBQSEL1.bit.GPIO39 = 2;  // 6 samples
    GpioCtrlRegs.GPBDIR.bit.GPIO42 = 0;    // input
    GpioCtrlRegs.GPBQSEL1.bit.GPIO42 = 2;  // 6 samples
    GpioCtrlRegs.GPBDIR.bit.GPIO43 = 0;    // input
    GpioCtrlRegs.GPBQSEL1.bit.GPIO43 = 2;  // 6 samples
    GpioCtrlRegs.GPBDIR.bit.GPIO44 = 0;    // input
    GpioCtrlRegs.GPBQSEL1.bit.GPIO44 = 2;  // 6 samples
    GpioCtrlRegs.GPBDIR.bit.GPIO45 = 0;    // input
    GpioCtrlRegs.GPBQSEL1.bit.GPIO45 = 2;  // 6 samples
    GpioCtrlRegs.GPBDIR.bit.GPIO46 = 0;    // input
    GpioCtrlRegs.GPBQSEL1.bit.GPIO46 = 2;  // 6 samples
    EDIS;

    DINT; // Disable global interrupt INTM

    InitPieCtrl(); // Init PIE control registers. All PIE interrupts disabled. All flags cleared.

    IER = 0x0000; // Disable individual CPU interrupts
    IFR = 0x0000; // Clear individual CPU interrupt flags

    InitPieVectTable(); // Init the PIE vector table with pointers to ISRs

    // Map ISR functions
    EALLOW;
    PieVectTable.ADCA1_INT = &adcA1ISR; // Function for ADCA interrupt 1
    EDIS;

    initADC(); // Configure the ADC and power it up
    initEPWM(); // Configure the ePWM
    initADCSOC(); // Setup the ADC for ePWM triggered conversions on channel 1

    // Enable global Interrupts and higher priority real-time debug events:
    IER |= M_INT1;  // Enable group 1 interrupts

    EINT;           // Enable global interrupt INTM
    ERTM;           // Enable global real time interrupt DBGM


    PieCtrlRegs.PIEIER1.bit.INTx1 = 1; // Enable PIE interrupt

    // Sync ePWM
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    while(1)
    {
        updateEPWM(cmp1, cmp2, phs);
        GpioDataRegs.GPATOGGLE.bit.GPIO22 = 1;
        DELAY_US(10000);

        /*
        for(cmp1 = 1; cmp1 < 2*PWM_PRD_QUARTER; cmp1++)
        {
            updateEPWM(PWM_PRD_QUARTER+cmp1, 3*PWM_PRD_QUARTER, 0);
            DELAY_US(1000);
        }

        for(cmp2 = 1; cmp2 < 2*PWM_PRD_QUARTER; cmp2++)
        {
            updateEPWM(3*PWM_PRD_QUARTER, 3*PWM_PRD_QUARTER-cmp2, 0);
            DELAY_US(1000);
        }

        for(phs = -PWM_PRD_QUARTER; phs < PWM_PRD_QUARTER; phs++)
        {
            updateEPWM(2*PWM_PRD_QUARTER, 2*PWM_PRD_QUARTER, phs);
            DELAY_US(1000);
        }
        */
    }
}

// initADC - Function to configure and power up ADCA.
void initADC(void)
{
    SetVREF(ADC_ADCA, ADC_INTERNAL, ADC_VREF3P3); // Setup VREF as internal

    EALLOW;
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6; // Set ADCCLK divider to /4
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1; // Set pulse positions to late
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1; // Power up the ADC and then delay for 1 ms
    EDIS;

    DELAY_US(1000);
}

// initEPWM - Function to configure ePWM1 to generate the SOC.
void initEPWM(void)
{
    EALLOW;
    // PWM1 configuration
    EPwm1Regs.ETSEL.bit.SOCAEN = 1;     // Disable SOC on A group
    EPwm1Regs.ETSEL.bit.SOCASEL = 1;    // Select SOC on up-count
    EPwm1Regs.ETPS.bit.SOCAPRD = 1;     // Generate pulse on 1st event

    EPwm1Regs.TBPRD = 2*PWM_PRD_HALF; // Set period to ~40kHz
    EPwm1Regs.CMPA.bit.CMPA = PWM_PRD_HALF; // Set compare A value to 50%
    EPwm1Regs.TBPHS.bit.TBPHS = 0;
    EPwm1Regs.TBCTR = 0;
    EPwm1Regs.TBCTL.bit.CTRMODE = 2;    // Up/down mode
    EPwm1Regs.TBCTL.bit.PHSEN = 0;      // Master module
    EPwm1Regs.EPWMSYNCOUTEN.bit.ZEROEN = 1;
    EPwm1Regs.TBCTL.bit.PRDLD = 0;      // Shadow register
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0;  // TBCLK = SYSCLK
    EPwm1Regs.TBCTL.bit.CLKDIV = 0;
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

    // PWM2 configuration
    EPwm2Regs.TBPRD = 2*PWM_PRD_HALF;// Set period to ~40kHz
    EPwm2Regs.CMPA.bit.CMPA = PWM_PRD_HALF; // Set compare A value to 50%
    EPwm2Regs.TBPHS.bit.TBPHS = 0;
    EPwm2Regs.TBCTR = 0;
    EPwm2Regs.TBCTL.bit.CTRMODE = 2;    // Up/down mode
    EPwm2Regs.TBCTL.bit.PHSEN = 1;      // Slave module
    EPwm2Regs.TBPHS.bit.TBPHS = 1*PWM_PRD_HALF; // 0.5π phase delay
    EPwm2Regs.TBCTL.bit.PRDLD = 0;      // Shadow register
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0;  // TBCLK = SYSCLK
    EPwm2Regs.TBCTL.bit.CLKDIV = 0;
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

    // PWM3 configuration
    EPwm3Regs.TBPRD = 2*PWM_PRD_HALF;// Set period to ~40kHz
    EPwm3Regs.CMPA.bit.CMPA = PWM_PRD_HALF; // Set compare A value to 50%
    EPwm3Regs.TBPHS.bit.TBPHS = 0;
    EPwm3Regs.TBCTR = 0;
    EPwm3Regs.TBCTL.bit.CTRMODE = 2;    // Up/down mode
    EPwm3Regs.TBCTL.bit.PHSEN = 1;      // Slave module
    EPwm3Regs.TBPHS.bit.TBPHS = 2*PWM_PRD_HALF; // π phase delay
    EPwm3Regs.TBCTL.bit.PRDLD = 0;      // Shadow register
    EPwm3Regs.TBCTL.bit.HSPCLKDIV = 0;  // TBCLK = SYSCLK
    EPwm3Regs.TBCTL.bit.CLKDIV = 0;
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

    // PWM4 configuration
    EPwm4Regs.TBPRD = 2*PWM_PRD_HALF;// Set period to ~40kHz
    EPwm4Regs.CMPA.bit.CMPA = PWM_PRD_HALF; // Set compare A value to 50%
    EPwm4Regs.TBPHS.bit.TBPHS = 0;
    EPwm4Regs.TBCTR = 0;
    EPwm4Regs.TBCTL.bit.CTRMODE = 2;    // Up/down mode
    EPwm4Regs.TBCTL.bit.PHSEN = 1;      // Slave module
    EPwm4Regs.TBPHS.bit.TBPHS = 3*PWM_PRD_HALF; // 1.5π phase delay
    EPwm4Regs.TBCTL.bit.PRDLD = 0;      // Shadow register
    EPwm4Regs.TBCTL.bit.HSPCLKDIV = 0;  // TBCLK = SYSCLK
    EPwm4Regs.TBCTL.bit.CLKDIV = 0;
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

    // PWM5 configuration
    EPwm5Regs.TBPRD = 2*PWM_PRD_HALF;// Set period to ~40kHz
    EPwm5Regs.CMPA.bit.CMPA = PWM_PRD_HALF; // Set compare A value to 50%
    EPwm5Regs.TBPHS.bit.TBPHS = 0;
    EPwm5Regs.TBCTR = 0;
    EPwm5Regs.TBCTL.bit.CTRMODE = 2;    // Up/down mode
    EPwm5Regs.TBCTL.bit.PHSEN = 1;      // Slave module
    EPwm5Regs.TBPHS.bit.TBPHS = 0*PWM_PRD_HALF; // 0 phase delay
    EPwm5Regs.TBCTL.bit.PRDLD = 0;      // Shadow register
    EPwm5Regs.TBCTL.bit.HSPCLKDIV = 0;  // TBCLK = SYSCLK
    EPwm5Regs.TBCTL.bit.CLKDIV = 0;
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

    // PWM6 configuration
    EPwm6Regs.TBPRD = 1248;             // Set period to ~40kHz
    EPwm6Regs.CMPA.bit.CMPA = PWM_PRD_HALF; // Set compare A value to 50%
    EPwm6Regs.TBPHS.bit.TBPHS = 0;
    EPwm6Regs.TBCTR = 0;
    EPwm6Regs.TBCTL.bit.CTRMODE = 2;    // Up/down mode
    EPwm6Regs.TBCTL.bit.PHSEN = 1;      // Slave module
    EPwm6Regs.TBPHS.bit.TBPHS = 1*PWM_PRD_HALF; // 0.5π phase delay
    EPwm6Regs.TBCTL.bit.PRDLD = 0;      // Shadow register
    EPwm6Regs.TBCTL.bit.HSPCLKDIV = 0;  // TBCLK = SYSCLK
    EPwm6Regs.TBCTL.bit.CLKDIV = 0;
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

    // PWM7 configuration
    EPwm7Regs.TBPRD = 2*PWM_PRD_HALF;// Set period to ~40kHz
    EPwm7Regs.CMPA.bit.CMPA = PWM_PRD_HALF; // Set compare A value to 50%
    EPwm7Regs.TBPHS.bit.TBPHS = 0;
    EPwm7Regs.TBCTR = 0;
    EPwm7Regs.TBCTL.bit.CTRMODE = 2;    // Up/down mode
    EPwm7Regs.TBCTL.bit.PHSEN = 1;      // Slave module
    EPwm7Regs.TBPHS.bit.TBPHS = 2*PWM_PRD_HALF; // π phase delay
    EPwm7Regs.TBCTL.bit.PRDLD = 0;      // Shadow register
    EPwm7Regs.TBCTL.bit.HSPCLKDIV = 0;  // TBCLK = SYSCLK
    EPwm7Regs.TBCTL.bit.CLKDIV = 0;
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

    /*
    // PWM8 configuration
    EPwm8Regs.TBPRD = 2*PWM_PRD_HALF;// Set period to ~40kHz
    EPwm8Regs.CMPA.bit.CMPA = 2*PWM_PRD_QUARTER; // Set compare A value to 50%
    EPwm8Regs.TBPHS.bit.TBPHS = 0;
    EPwm8Regs.TBCTR = 0;
    EPwm8Regs.TBCTL.bit.CTRMODE = 2;    // Up/down mode
    EPwm8Regs.TBCTL.bit.PHSEN = 1;      // Slave module
    EPwm8Regs.TBPHS.bit.TBPHS = 6*PWM_PRD_QUARTER; // 1.5π phase delay
    EPwm8Regs.TBCTL.bit.PRDLD = 0;      // Shadow register
    EPwm8Regs.TBCTL.bit.HSPCLKDIV = 0;  // TBCLK = SYSCLK
    EPwm8Regs.TBCTL.bit.CLKDIV = 0;
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
    */
    EDIS;
}

// initEPWM - Function to configure ePWM1 to generate the SOC.
void updateEPWM(Uint16 cmpIn, Uint16 cmpOut, int16 phaseShiftInOut)
{
    // Set compare A values for input bridge
    EPwm1Regs.CMPA.bit.CMPA = cmpIn;
    EPwm2Regs.CMPA.bit.CMPA = cmpIn;
    EPwm3Regs.CMPA.bit.CMPA = cmpIn;
    EPwm4Regs.CMPA.bit.CMPA = cmpIn;

    // Set compare A values for output bridge
    EPwm5Regs.CMPA.bit.CMPA = cmpOut;
    EPwm6Regs.CMPA.bit.CMPA = cmpOut;
    EPwm7Regs.CMPA.bit.CMPA = cmpOut;
    //EPwm8Regs.CMPA.bit.CMPA = cmpOut;

    // Set phase delay for output bridge
    EPwm5Regs.TBPHS.bit.TBPHS = 0*PWM_PRD_HALF + phaseShiftInOut; //    π + Θ phase delay
    EPwm6Regs.TBPHS.bit.TBPHS = 1*PWM_PRD_HALF + phaseShiftInOut; // 0.5π + Θ phase delay
    EPwm7Regs.TBPHS.bit.TBPHS = 2*PWM_PRD_HALF + phaseShiftInOut; //   1π + Θ phase delay
    //EPwm8Regs.TBPHS.bit.TBPHS = 6*PWM_PRD_QUARTER + phaseShiftInOut; // 1.5π + Θ phase delay
}

// initADCSOC - Function to configure ADCA's SOCs to be triggered by ePWM1
void initADCSOC(void)
{
    // Select the channels to convert and the end of conversion flag
    EALLOW;
    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;     // SOC0 will convert pin A0 = Vin
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = 9;     // Sample window is 10 SYSCLK cycles
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 5;   // Trigger on ePWM1 SOCA

    AdcaRegs.ADCSOC1CTL.bit.CHSEL = 1;     // SOC1 will convert pin A1 = Vout
    AdcaRegs.ADCSOC1CTL.bit.ACQPS = 9;     // Sample window is 10 SYSCLK cycles
    AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = 5;   // Trigger on ePWM1 SOCA

    AdcaRegs.ADCSOC2CTL.bit.CHSEL = 2;     // SOC2 will convert pin A2 = Vclamp
    AdcaRegs.ADCSOC2CTL.bit.ACQPS = 9;     // Sample window is 10 SYSCLK cycles
    AdcaRegs.ADCSOC2CTL.bit.TRIGSEL = 5;   // Trigger on ePWM1 SOCA

    AdcaRegs.ADCSOC3CTL.bit.CHSEL = 3;     // SOC3 will convert pin A3 = Iout
    AdcaRegs.ADCSOC3CTL.bit.ACQPS = 9;     // Sample window is 10 SYSCLK cycles
    AdcaRegs.ADCSOC3CTL.bit.TRIGSEL = 5;   // Trigger on ePWM1 SOCA

    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 3; // End of SOC3 will set INT1 flag
    AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;   // Enable INT1 flag
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // Make sure INT1 flag is cleared
    EDIS;
}

// adcA1ISR - ADC A Interrupt 1 ISR
__interrupt void adcA1ISR(void)
{
    // GpioDataRegs.GPATOGGLE.bit.GPIO22 = 1;
    // ADCRESULT0 is the result register of SOC0
    // AdcaResultRegs.ADCRESULT0;

    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // Clear the interrupt flag

    // Check if overflow has occurred
    if(1 == AdcaRegs.ADCINTOVF.bit.ADCINT1)
    {
        AdcaRegs.ADCINTOVFCLR.bit.ADCINT1 = 1; //clear INT1 overflow flag
        AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // Acknowledge the interrupt
}
