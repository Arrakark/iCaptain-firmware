#include "Autopilot.h"

SoftwareSerial gps_serial = SoftwareSerial(13, 15, false, 256);
SoftwareSerial bt_serial = SoftwareSerial(12, 14, false, 256);
SSD1306 display(0x3c, D2, D1);
TinyGPSPlus gps;

autopilot::autopilot()
{
    state = 1;
    heading = 0;
    rudder_manual = 0;
    waypoint_long = 0;
    waypoint_lat = 0;
}

void autopilot::init()
{
    //begin serial connections
    gps_serial.begin(9600);
    bt_serial.begin(9600);
    //init display and say hello!
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 24, "iCaptain Autopilot");
    display.display();
    delay(2000);
}

void autopilot::update_display()
{
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    if (state == 0)
    {
        display.drawString(64, 0, "ERROR");
    }
    else if (state == 1)
    {
        display.drawString(64, 0, "OFFLINE");
    }
    else if (state == 2)
    {
        display.drawString(64, 0, "MANUAL");
    }
    else if (state == 3)
    {
        display.drawString(64, 0, "AUTOPILOT");
    }
    if (gps.speed.isValid() && gps.course.isValid() && gps.location.isValid())
    {
        display.setFont(ArialMT_Plain_10);
        display.drawString(64, 16, String("Course: ") + String(gps.course.deg(), 6));
        display.drawString(64, 26, String("Speed (knots): ") + String(gps.speed.knots(), 6));
    }
    else
    {
        display.setFont(ArialMT_Plain_16);
        if (!gps.course.isValid())
        {
            display.drawString(64, 16, "Invalid Course");
        }
        else if (!gps.speed.isValid())
        {
            display.drawString(64, 16, "Invalid Speed");
        }
        else if (!gps.location.isValid())
        {
            display.drawString(64, 16, "Invalid Location");
        }
        else
        {
            display.drawString(64, 16, "No GPS Lock");
        }
    }
    display.display();
}

void autopilot::update_gps()
{
    while (gps_serial.available() > 0)
        gps.encode(gps_serial.read());

    if (millis() > 5000 && gps.charsProcessed() < 10)
    {
        state = 0;
    }
}

void autopilot::set_state(int new_state)
{
    //setting state to autopilot
    if (new_state == 3)
    {
        if (gps.location.isValid() && gps.course.isValid())
        {
            waypoint_lat = gps.location.lat();
            waypoint_long = gps.location.lng();
            heading = gps.course.deg();
            state = 3;
        }
        else
        {
            //do not have GPS lock; could not enable autopilot
            state = 1;
        }
    }
    //setting state to manual control mode
    if (new_state = 2)
    {
        rudder_manual = 0;
        state = 1;
    }
    //setting state to OFFLINE
    if (new_state == 1)
    {
        state = 1;
    }
}

void autopilot::set_heading(int new_heading)
{
    heading = new_heading;
}
void autopilot::set_rudder(int new_rudder)
{
    rudder_manual = new_rudder;
}