
#include "f28x_project.h"
#include "controller.h"
#include "init.h"
#include "pwm.h"
#include "adc.h"
#include "leds.h"
#include "input.h"
#include "relay.h"

// Globals
static enum converter_states converter_state = StateInitDSP;
enum trip_status tripStatus = NoTrip;

// Functions
void adjust_reference(enum button);

// Main
void main(void)
{
    while(1)
    {
        enum button button = button_pressed();

        converter_state = (tripStatus == NoTrip) ? converter_state : StateTrip;

        switch (converter_state)
        {
        case StateInitDSP:
        {
            initDSP(); // Configure GPIO, ADC, PWM
            initTripFeedback(&tripStatus);
            initPIConttrollers(); // Initialize PI controllers
            setControllerIoutRef(0.0);
            setControllerVclampRef(0.0);
            converter_state = StateStandby;
            break;
        }
        case StateStandby:
        {
            adjust_reference(button);
            // Enable startup transition only if the converter is within SOA
            if (button == BtnOn /*&& isInSOA(readADC(), StateStandby) == NoTrip*/)
            {
                converter_state = StateStartup;
            }
            break;
        }
        case StateStartup:
        {
            relayOn();
            initPIConttrollers();
            enablePWM();
            converter_state = StateOn;
            break;
        }
        case StateOn:
        {
            adjust_reference(button);
            converter_state = button == BtnOff ? StateShutdown : converter_state;

            break;
        }
        case StateShutdown:
        {
            relayOff();
            disablePWM();
            converter_state = StateStandby;
            break;
        }
        case StateTrip:
        {
            relayOff(); // TODO Is this safe e.g. in over-current condition?

            // Indicate trip status on LEDs
            if (tripStatus == TripOC) { ledOn(LEDTripOC); }
            if (tripStatus == TripSOAVin) { ledOn(LEDTripSOAVin); }
            if (tripStatus == TripSOAVout) { ledOn(LEDTripSOAVout); }
            if (tripStatus == TripSOAVclamp) { ledOn(LEDTripSOAVclamp); }

            // Clear trip condition only if trip clear button is pressed and the converter is within SOA
            if (button == BtnClrTrip /*&& isInSOA(readADC(), StateStandby) == NoTrip*/)
            {
                tripStatus = NoTrip;
                converter_state = StateStandby;
                ledsOff(); // Turn off status LEDs
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

void adjust_reference(enum button button)
{
    static enum parameter param = Iout;

    if (button == BtnSelectRef) { param = (param == Iout) ? Vclamp : Iout; } // switch between Iref and

    if (param == Iout && button == BtnIncr) { adjControllerIoutRef(+0.5); }
    if (param == Iout && button == BtnDecr) { adjControllerIoutRef(-0.5); }
    if (param == Vclamp && button == BtnIncr) { adjControllerIoutRef(+1.0); }
    if (param == Vclamp && button == BtnDecr) { adjControllerIoutRef(-1.0); }

    // TODO display reference parameter and Iout, Vclamp reference values values

    return;
}
