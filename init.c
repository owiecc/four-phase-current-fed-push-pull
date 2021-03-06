
#include "init.h"
#include "f28x_project.h"
#include "f28003x_device.h"
#include "controller.h"
#include "pwm.h"
#include "adc.h"

void initDSP(void)
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

    // Soft-start relay
    GpioCtrlRegs.GPBPUD.bit.GPIO34 = 0;  // Relay on
    GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0; // GPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;  // output

    // Button inputs
    GpioCtrlRegs.GPBCTRL.bit.QUALPRD0 = 1; // Qual period = SYSCLKOUT/2
    GpioCtrlRegs.GPBDIR.bit.GPIO40 = 0;    // input
    GpioCtrlRegs.GPBQSEL1.bit.GPIO40 = 2;  // 6 samples
    GpioCtrlRegs.GPBDIR.bit.GPIO49 = 0;    // input
    GpioCtrlRegs.GPBQSEL2.bit.GPIO49 = 2;  // 6 samples
    GpioCtrlRegs.GPBDIR.bit.GPIO51 = 0;    // input
    GpioCtrlRegs.GPBQSEL2.bit.GPIO51 = 2;  // 6 samples
    GpioCtrlRegs.GPBDIR.bit.GPIO48 = 0;    // input
    GpioCtrlRegs.GPBQSEL2.bit.GPIO48 = 2;  // 6 samples
    GpioCtrlRegs.GPBDIR.bit.GPIO50 = 0;    // input
    GpioCtrlRegs.GPBQSEL2.bit.GPIO50 = 2;  // 6 samples
    GpioCtrlRegs.GPBDIR.bit.GPIO52 = 0;    // input
    GpioCtrlRegs.GPBQSEL2.bit.GPIO52 = 2;  // 6 samples
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
}
