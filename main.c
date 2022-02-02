
#include "f28002x_device.h"
#include "f28x_project.h"
#include "pwm.h"

// Defines
#define PWM_PRD_HALF     625 // Quarter of a switching period ~40kHz

// Function prototypes
void initADC(void);
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
    initEPWM(PWM_PRD_HALF); // Configure the ePWM
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
        // updateEPWM(cmp1, cmp2, phs);
        for(phs = -2*PWM_PRD_HALF; phs < 2*PWM_PRD_HALF; phs++)
        {
            updateEPWM(cmp1, cmp2, phs);
            DELAY_US(1000);
        }
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
