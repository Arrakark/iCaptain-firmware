#include "Autopilot.h"

SSD1306 display(0x3c, D2, D1);
TinyGPSPlus gps;
int autopilot::state = 1;
float autopilot::heading = 0;
double autopilot::waypoint_long = 0;
double autopilot::waypoint_lat = 0;
double autopilot::lat = 0;
double autopilot::lng = 0;
double autopilot::hdop = 0;
double autopilot::course = 0;
double autopilot::speed = 0;
double autopilot::p_gain = 1.0;
double autopilot::d_gain = 1.0;
int autopilot::sat_count = 0;
float autopilot::rudder_speed = 0;
long autopilot::last_manual_message = 0;
float autopilot::cross_track_error = 0;
float autopilot::direction_error = 0;
AccelStepper autopilot::motor = AccelStepper(AccelStepper::DRIVER, PULSE, DIR);

void autopilot::init()
{
    //begin serial connections
    Serial.begin(9600);
    Serial.swap();
    motor.setMinPulseWidth(20);
    motor.setAcceleration(100);
    motor.setMaxSpeed(100);
    motor.setEnablePin(ENABLE);
    motor.setPinsInverted(false,false,true);

    //init display and say hello!
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 24, "iCaptain Autopilot");
    display.display();
    delay(500);
    motor.disableOutputs();
}

void autopilot::update_display()
{
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    yield();
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
    display.setFont(ArialMT_Plain_10);
    yield();
    if (gps.location.isValid())
    {
        display.drawString(64, 16, String(gps.location.lat(), 4) + " " + String(gps.location.lng(), 4));
    }
    else
    {
        display.drawString(64, 16, "NO GPS LOCK");
    }
    String second_line = String();
    yield();
    if (gps.hdop.isValid())
    {
        second_line = "HD:" + String(gps.hdop.hdop(), 1);
    }
    else
    {
        second_line = "-";
    }
    if (gps.course.isValid())
    {
        second_line = second_line + " H:" + String((int)gps.course.deg());
    }
    else
    {
        second_line = second_line + " -";
    }
    if (gps.speed.isValid())
    {
        second_line = second_line + " S:" + String(gps.speed.knots(), 1);
    }
    else
    {
        second_line = second_line + " -";
    }
    display.drawString(64, 26, second_line);
    String third_line = String("Rudder: ");
    third_line = third_line + String(rudder_speed, 0);
    display.drawString(64, 36, third_line);
    display.display();
}

void autopilot::update_gps()
{
    while (Serial.available() > 0)
    {
        yield();
        run_motor();
        gps.encode(Serial.read());
    }

    if (millis() > 5000 && gps.charsProcessed() < 10)
    {
        state = 0;
    }
}

void autopilot::update_values()
{
    if (gps.location.isValid())
    {
        lat = gps.location.lat();
        lng = gps.location.lng();
    }

    if (gps.course.isValid())
    {
        course = gps.course.deg();
    }
    if (gps.course.isValid())
    {
        course = gps.course.deg();
    }
    if (gps.speed.isValid())
    {
        speed = gps.speed.knots();
    }
    if (gps.hdop.isValid())
    {
        hdop = gps.hdop.hdop();
    }
    sat_count = gps.satellites.value();
}

int autopilot::set_state(String command)
{
    int new_state = command.toInt();
    rudder_speed = 0;
    //setting state to autopilot
    if (new_state == 3)
    {
        //change this back!
        if (gps.location.isValid() && gps.course.isValid() && gps.speed.knots() > 2 && gps.hdop.value() <= 7)
        {
            waypoint_lat = gps.location.lat();
            waypoint_long = gps.location.lng();
            heading = gps.course.deg();
            state = 3;
            motor.enableOutputs();
            return 1;
        }
        else
        {
            //do not have GPS lock; could not enable autopilot
            //state = 1;
            return 0;
        }
    }
    //setting state to manual control mode
    if (new_state == 2)
    {
        state = 2;
        motor.enableOutputs();
        return 1;
    }
    //setting state to OFFLINE
    if (new_state == 1)
    {
        state = 1;
        motor.disableOutputs();
        return 1;
    }
    return 0;
}

int autopilot::set_heading(String command)
{
    int new_heading = command.toInt();
    if (new_heading >= 0 && new_heading <= 360 && gps.hdop.value() <= 7)
    {
        heading = new_heading;
        waypoint_lat = gps.location.lat();
        waypoint_long = gps.location.lng();
        return 1;
    }
    return 0;
}

int autopilot::manual_control(String command)
{
    int manual_command = command.toInt();
    if (manual_command >= -100 && manual_command <= 100 && state == 2)
    {
        last_manual_message = millis();
        rudder_speed = manual_command;
        return 1;
    }
    return 0;
}

void autopilot::pid_control()
{
    //autopilot mode
    if (state == 3)
    {
        //control via PID
        if (gps.hdop.value() >= 7){
            //have GPS lock, proceed to autopilot
            calculate_cross_track_error();
            calculate_direction_error();
            float p_error = p_gain * cross_track_error;
            float d_error = d_gain * direction_error;
            //limit boat from spinning in circles if you are too far away from waypoint
            if (p_error > 50)
                p_error = 50;
            if (p_error < -50)
                p_error = -50;
            rudder_speed = -1 * (p_error + d_error);
            if (rudder_speed > 100)
                rudder_speed = 100;
            if (rudder_speed < -100)
                rudder_speed = -100;
        }
        else {
            rudder_speed = 0;
        }
    }
    else if (state == 2)
    {
        //control rudder speed manually
        //check for timeout here
        if (millis() - last_manual_message > 1000)
        {
            rudder_speed = 0;
        }
    }
    else
    {
        //do not move rudder at all
        rudder_speed = 0;
    }
    motor.setSpeed(rudder_speed);
}

/*
 *  Calculates and saves cross-track error (units are meters)
 *  to the right of the line: positive
 *  to the left of the line: negative
 */
void autopilot::calculate_cross_track_error()
{
    float theta12 = heading * (PI / 180.0);
    float theta13 = TinyGPSPlus::courseTo(waypoint_lat, waypoint_long, lat, lng) * (PI / 180);
    float d13 = TinyGPSPlus::distanceBetween(waypoint_lat, waypoint_long, lat, lng) / R;
    cross_track_error = asin(sin(d13) * sin(theta13 - theta12)) * R;
}

void autopilot::calculate_direction_error() 
{
    int i = 0;
    if (course > heading)
        i = course - heading;
    else
        i = heading - course;
    if (i > 180)
        i = 360 - i;
    if (heading - course > 180 || heading - course < 0)
        i = -i;
    direction_error = i;
}

void autopilot::run_motor(){
    motor.runSpeed();
}
