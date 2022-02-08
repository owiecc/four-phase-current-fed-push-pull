
#include "f28x_project.h"
#include "controller.h"
#include "init.h"
#include "pwm.h"
#include "adc.h"
#include "leds.h"
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
            // Indicate trip status on LEDs
            if (tripStatus == TripOC) { ledOn(LEDTripOC); } else { ledOff(LEDTripOC); }
            if (tripStatus == TripSOAVin) { ledOn(LEDTripSOAVin); } else { ledOff(LEDTripSOAVin); }
            if (tripStatus == TripSOAVout) { ledOn(LEDTripSOAVout); } else { ledOff(LEDTripSOAVout); }
            if (tripStatus == TripSOAVclamp) { ledOn(LEDTripSOAVclamp); } else { ledOff(LEDTripSOAVclamp); }

            // Clear trip condition only if trip clear button is pressed and the converter is within SOA
            if (button_pressed() == BtnClrTrip && isInSOA(readADC(), StateStandby) == NoTrip)
            {
                tripStatus = NoTrip;
                converter_state = StateStandby;
            }
        }
        default:
        {

        }
        }
        DELAY_US(200000); // 0.2s
    }
}
