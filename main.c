
#include "f28002x_device.h"
#include "f28x_project.h"

// Defines
#define RESULTS_BUFFER_SIZE     256

// Globals
uint16_t adcAResults[RESULTS_BUFFER_SIZE];   // Buffer for results
uint16_t index;                              // Index into result buffer
volatile uint16_t bufferFull;                // Flag to indicate buffer is full

// Function prototypes
void initADC(void);
void initEPWM(void);
void initADCSOC(void);
__interrupt void adcA1ISR(void);

// Main
void main(void)
{
    InitSysCtrl(); // Init device clock and peripherals
    InitGpio(); // Init GPIO

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

    // Initialize results buffer
    for(index = 0; index < RESULTS_BUFFER_SIZE; index++)
    {
        adcAResults[index] = 0;
    }
    index = 0;
    bufferFull = 0;

    PieCtrlRegs.PIEIER1.bit.INTx1 = 1; // Enable PIE interrupt

    // Sync ePWM
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    while(1)
    {
        // Start ePWM
        EPwm1Regs.ETSEL.bit.SOCAEN = 1;    // Enable SOCA
        EPwm1Regs.TBCTL.bit.CTRMODE = 0;   // Un-freeze, and enter up count mode

        // Wait while ePWM causes ADC conversions, which then cause interrupts,
        // which fill the results buffer, eventually setting the bufferFull flag
        while(!bufferFull)
        {
        }
        bufferFull = 0; //clear the buffer full flag

        // Stop ePWM
        EPwm1Regs.ETSEL.bit.SOCAEN = 0;    // Disable SOCA
        EPwm1Regs.TBCTL.bit.CTRMODE = 3;   // Freeze counter

        // Software breakpoint. At this point, conversion results are stored in adcAResults.
        ESTOP0; // Hit run again to get updated conversions.
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
    EPwm1Regs.ETSEL.bit.SOCAEN = 0;     // Disable SOC on A group
    EPwm1Regs.ETSEL.bit.SOCASEL = 4;    // Select SOC on up-count
    EPwm1Regs.ETPS.bit.SOCAPRD = 1;     // Generate pulse on 1st event

    EPwm1Regs.CMPA.bit.CMPA = 0x0800;   // Set compare A value to 2048 counts
    EPwm1Regs.TBPRD = 0x1000;           // Set period to 4096 counts

    EPwm1Regs.TBCTL.bit.CTRMODE = 3;    // Freeze counter
    EDIS;
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
    // Add the latest result to the buffer
    // ADCRESULT0 is the result register of SOC0
    adcAResults[index++] = AdcaResultRegs.ADCRESULT0;

    // Set the bufferFull flag if the buffer is full
    if(RESULTS_BUFFER_SIZE <= index)
    {
        index = 0;
        bufferFull = 1;
    }

    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // Clear the interrupt flag

    // Check if overflow has occurred
    if(1 == AdcaRegs.ADCINTOVF.bit.ADCINT1)
    {
        AdcaRegs.ADCINTOVFCLR.bit.ADCINT1 = 1; //clear INT1 overflow flag
        AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // Acknowledge the interrupt
}
