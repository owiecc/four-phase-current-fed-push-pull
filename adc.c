
#include "adc.h"
#include "f28x_project.h"

const int N_AVG_IOUT_CAL = 128;

struct ADCCalibration ADCCal = {
    .coeffACD0 = (struct ADCScaling) {.gain = (9*665e3+10e3/2+10e3)/10e3*3.3/4095, .offset = 0}, // (9×665k + 10k/2, 10k) voltage divider
    .coeffACD1 = (struct ADCScaling) {.gain = (6*665e3+10e3)/10e3*3.3/4095, .offset = 0}, // (6×665k, 10k) voltage divider
    .coeffACD2 = (struct ADCScaling) {.gain = (6*665e3+10e3)/10e3*3.3/4095, .offset = 0}, // (6×665k, 10k) voltage divider
    .coeffACD3 = (struct ADCScaling) {.gain = -0.012, .offset = 1890} // LEM6-NP + (3.3k, 6.8k) voltage divider; calibrated parameters
};

void calibrateADC(void)
{
    DELAY_US(1000000); // 1.0s

    unsigned long int IoutOffset = 0;
    for (int i = 0; i < N_AVG_IOUT_CAL; i++)
    {
        DELAY_US(320000/N_AVG_IOUT_CAL); // total process should take multiple of 20ms (works in 50/60Hz grid)
        readADC();
        IoutOffset += AdcaResultRegs.ADCRESULT3; // Iout ADC value
    }

    ADCCal.coeffACD3.offset = (int)(IoutOffset/N_AVG_IOUT_CAL);
}

struct ADCResult readADC(void)
{
    // Do not trigger ADCINT1 if PWMs are disabled; force trigger the ADC conversion
    if (EPwm1Regs.TBCTL.bit.CTRMODE == TB_FREEZE)
    {
        EALLOW;
        //AdcaRegs.ADCINTSEL1N2.bit.INT1E = 0;   // Disable ADCINT1

        AdcaRegs.ADCSOCFRC1.all = 0x000F; // Force SOC0 to SOC3
        DELAY_US(1);

        //AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;   // Enable ADCINT1
        AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // Make sure ADCINT1 flag is cleared
        EDIS;
    }

    struct ADCResult adcOut;
    adcOut = scaleADCs();

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
    return ((int)ADCResult - coeffADC.offset)*coeffADC.gain;
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
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = 24;     // Sample window is 25 SYSCLK cycles
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 5;   // Trigger on ePWM1 SOCA

    AdcaRegs.ADCSOC1CTL.bit.CHSEL = 1;     // SOC1 will convert pin A1 = Vin
    AdcaRegs.ADCSOC1CTL.bit.ACQPS = 24;     // Sample window is 25 SYSCLK cycles
    AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = 5;   // Trigger on ePWM1 SOCA

    AdcaRegs.ADCSOC2CTL.bit.CHSEL = 2;     // SOC2 will convert pin A2 = Vout
    AdcaRegs.ADCSOC2CTL.bit.ACQPS = 24;     // Sample window is 25 SYSCLK cycles
    AdcaRegs.ADCSOC2CTL.bit.TRIGSEL = 5;   // Trigger on ePWM1 SOCA

    AdcaRegs.ADCSOC3CTL.bit.CHSEL = 4;     // SOC3 will convert pin A4 = Iout
    AdcaRegs.ADCSOC3CTL.bit.ACQPS = 24;     // Sample window is 25 SYSCLK cycles
    AdcaRegs.ADCSOC3CTL.bit.TRIGSEL = 5;   // Trigger on ePWM1 SOCA

    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 3; // End of SOC3 will set ADCINT1 flag
    AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;   // Enable ADCINT1
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // Make sure ADCINT1 flag is cleared
    EDIS;
}
