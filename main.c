
#include "f28x_project.h"
#include "controller.h"
#include "init.h"
#include "pwm.h"
#include "adc.h"
#include "leds.h"
#include "input.h"
#include "relay.h"

// Globals
static enum converter_states converter_state;
enum trip_status tripStatus;

// Main
void main(void)
{
    converter_state = StateInitDSP;
    while(1)
    {
        enum button button = button_pressed();

        switch (converter_state)
        {
        case StateInitDSP:
        {
            initDSP(); // Configure GPIO, ADC, PWM
            initPIConttrollers(); // Initialize PI controllers
            // TODO Turn the Vclamp controller off
            // TODO Turn the Iout controller off
            converter_state = StateStandby;
            break;
        }
        case StateStandby:
        {
            struct ADCResult sensors;
            sensors = readADC();

            // Enable startup transition only if the converter is within SOA
            if (button == BtnOn && isInSOA(readADC(), StateStandby) == NoTrip)
            {
                converter_state = StateStartup;
            }
            break;
        }
        case StateStartup:
        {
            relayOn();
            // TODO Turn the Vclamp controller on
            break;
        }
        case StateOn:
        {
            // TODO Turn the Iout controller on
            // TODO Set reference current
            break;
        }
        case StateShutdown:
        {
            relayOff();
            // TODO Set reference current to zero
            break;
        }
        case StateTrip:
        {
            relayOff(); // TODO Is this safe e.g. in over-current condition?

            // Indicate trip status on LEDs
            if (tripStatus == TripOC) { ledOn(LEDTripOC); } else { ledOff(LEDTripOC); }
            if (tripStatus == TripSOAVin) { ledOn(LEDTripSOAVin); } else { ledOff(LEDTripSOAVin); }
            if (tripStatus == TripSOAVout) { ledOn(LEDTripSOAVout); } else { ledOff(LEDTripSOAVout); }
            if (tripStatus == TripSOAVclamp) { ledOn(LEDTripSOAVclamp); } else { ledOff(LEDTripSOAVclamp); }

            // Clear trip condition only if trip clear button is pressed and the converter is within SOA
            if (button == BtnClrTrip && isInSOA(readADC(), StateStandby) == NoTrip)
            {
                tripStatus = NoTrip;
                converter_state = StateStandby;
            }
            break;
        }
        default:
        {
            break;
        }
        }
        DELAY_US(200000); // 0.2s
    }
}
