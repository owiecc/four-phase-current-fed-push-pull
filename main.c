
#include "f28x_project.h"
#include "controller.h"
#include "init.h"
#include "pwm.h"
#include "adc.h"
#include "input.h"

// Globals
static enum converter_states converter_state;
enum trip_reasons tripStatus;

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

            // Enable startup transition only if all voltages are correct
            tripStatus = isInSOA(sensors, StateStandby);
            if (tripStatus == NoTrip) { break; }
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
        case StateTrip:
        {

        }
        default:
        {

        }
        }
        DELAY_US(200000); // 0.2s
    }
}
