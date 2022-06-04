
#include "adc.h"
#include "f28x_project.h"

struct ADCCalibration ADCCal = {
    .coeffACD0 = (struct ADCScaling) {.gain = (9*665e3+10e3/2+10e3)/10e3*3.3/4095, .offset = 0}, // (9×665k + 10k/2, 10k) voltage divider
    .coeffACD1 = (struct ADCScaling) {.gain = (6*665e3+10e3)/10e3*3.3/4095, .offset = 0}, // (6×665k, 10k) voltage divider
    .coeffACD2 = (struct ADCScaling) {.gain = (6*665e3+10e3)/10e3*3.3/4095, .offset = 0}, // (6×665k, 10k) voltage divider
    //.coeffACD3 = (struct ADCScaling) {.gain = 6.0/0.625*(3.3e3+6.8e3)/6.8e3*3.3/4095, .offset = (unsigned int) 2.5*6.8e3/(3.3e3+6.8e3)*4095/3.3} // LEM6-NP + (3.3k, 6.8k) voltage divider
    .coeffACD3 = (struct ADCScaling) {.gain = 0.012876152, .offset = 1857} // LEM6-NP + (3.3k, 6.8k) voltage divider
};

struct ADCResult readADC(void)
{
    AdcaRegs.ADCSOCFRC1.all = 0x000F; // Force SOC0 to SOC3

    while (AdcaRegs.ADCINTFLG.bit.ADCINT1 != 1) {} // Wait for conversion to finish

    struct ADCResult adcOut;
    adcOut = scaleADCs();

    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // Clear the interrupt flag

    return adcOut;
}

inline struct ADCResult scaleADCs(void)
{
    struct ADCResult adcOut;
    adcOut.Vclamp = scaleADC(AdcaResultRegs.ADCRESULT0, ADCCal.coeffACD0);
    adcOut.Vin = scaleADC(AdcaResultRegs.ADCRESULT1, ADCCal.coeffACD1);
    adcOut.Vout = scaleADC(AdcaResultRegs.ADCRESULT2, ADCCal.coeffACD2);
    adcOut.Iout = scaleADC(AdcaResultRegs.ADCRESULT3, ADCCal.coeffACD3);

    return adcOut;
}

inline float scaleADC(unsigned int ADCResult, struct ADCScaling coeffADC)
{
    return (ADCResult - coeffADC.offset)*coeffADC.gain;
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
    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;     // SOC0 will convert pin A0 = Vclamp
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = 9;     // Sample window is 10 SYSCLK cycles
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 5;   // Trigger on ePWM1 SOCA

    AdcaRegs.ADCSOC1CTL.bit.CHSEL = 1;     // SOC1 will convert pin A1 = Vin
    AdcaRegs.ADCSOC1CTL.bit.ACQPS = 9;     // Sample window is 10 SYSCLK cycles
    AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = 5;   // Trigger on ePWM1 SOCA

    AdcaRegs.ADCSOC2CTL.bit.CHSEL = 2;     // SOC2 will convert pin A2 = Vout
    AdcaRegs.ADCSOC2CTL.bit.ACQPS = 9;     // Sample window is 10 SYSCLK cycles
    AdcaRegs.ADCSOC2CTL.bit.TRIGSEL = 5;   // Trigger on ePWM1 SOCA

    AdcaRegs.ADCSOC3CTL.bit.CHSEL = 4;     // SOC3 will convert pin A4 = Iout
    AdcaRegs.ADCSOC3CTL.bit.ACQPS = 9;     // Sample window is 10 SYSCLK cycles
    AdcaRegs.ADCSOC3CTL.bit.TRIGSEL = 5;   // Trigger on ePWM1 SOCA

    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 3; // End of SOC3 will set INT1 flag
    AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;   // Enable INT1 flag
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // Make sure INT1 flag is cleared
    EDIS;
}
