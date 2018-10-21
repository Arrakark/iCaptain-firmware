#include <Arduino.h>
#include <Autopilot.h>
#include <AccelStepper.h>

#define PULSE D3
#define DIR 10
#define ENABLE 9

AccelStepper stepper = AccelStepper(AccelStepper::DRIVER, PULSE, DIR);

autopilot iCaptain = autopilot();

void setup()
{
    iCaptain.init();
    Serial.begin(9600);
}

void loop()
{
    iCaptain.update_display();
    iCaptain.update_gps();
}