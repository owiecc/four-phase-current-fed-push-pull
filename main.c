
#include "f28x_project.h"
#include "controller.h"
#include "init.h"
#include "pwm.h"
#include "adc.h"
#include "input.h"

// Globals
enum converter_states {StateInitDSP,StateStandby,StateStartup,StateOn,StateShutdown,StateFail};
static enum converter_states converter_state;

// Main
void main(void)
{
    while(1)
    {
        converter_state = StateInitDSP;
        // Get ADC values
        // Get button pressed

        switch (converter_state)
        {
        case StateInitDSP:
        {
            initDSP(); // Configure GPIO, ADC, PWM
            converter_state = StateStandby;
        }
        case StateStandby:
        {
            struct ADCResult sensors;
            sensors = readADC();
            sensors.Vin =  sensors.Vin + 0.0001;
            // Enable startup transition only if all voltages are correct
        }
        case StateStartup:
        {
            // Turn the Vclamp controller on
            // Turn the current controller off
        }
        case StateOn:
        {
            // Set reference current
        }
        case StateShutdown:
        {

        }
        case StateFail:
        {

        }
        default:
        {

        }
        }
        DELAY_US(200000); // 0.2s
    }
}
