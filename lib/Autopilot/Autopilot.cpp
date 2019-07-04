#include "Autopilot.h"

// SSD1306 display(0x3c, D2, D1);
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
double autopilot::p_gain = -0.75;
double autopilot::dd_gain = -20;
double autopilot::d_gain = 0.75;
bool autopilot::rudder_engaged = false;
int autopilot::sat_count = 0;
float autopilot::rudder_control = 0;
float autopilot::rudder_position = 0;
long autopilot::last_manual_message = 0;
long autopilot::last_error_calculation = 0;
float autopilot::dd_error = 0;
float autopilot::cross_track_error = 0;
float autopilot::direction_error = 0;

void autopilot::init()
{
    //begin serial connections
    //pinMode(SENSOR,INPUT);

    //init display and say hello!
    // display.init();
    // display.flipScreenVertically();
    // display.setFont(ArialMT_Plain_16);
    // display.setTextAlignment(TEXT_ALIGN_CENTER);
    // display.drawString(64, 24, "iCaptain Autopilot");
    // display.display();
    // delay(500);
    rudder_engaged = false;
}

void autopilot::update_display()
{
    // display.clear();
    // display.setFont(ArialMT_Plain_16);
    // display.setTextAlignment(TEXT_ALIGN_CENTER);
    // yield();
    // if (state == 0)
    // {
    //     display.drawString(64, 0, "ERROR");
    // }
    // else if (state == 1)
    // {
    //     display.drawString(64, 0, "OFFLINE");
    // }
    // else if (state == 2)
    // {
    //     display.drawString(64, 0, "MANUAL");
    // }
    // else if (state == 3)
    // {
    //     display.drawString(64, 0, "AUTOPILOT");
    // }
    // display.setFont(ArialMT_Plain_10);
    // yield();
    // if (gps.location.isValid())
    // {
    //     display.drawString(64, 16, String(gps.location.lat(), 4) + " " + String(gps.location.lng(), 4));
    // }
    // else
    // {
    //     display.drawString(64, 16, "NO GPS LOCK");
    // }
    // String second_line = String();
    // yield();
    // if (gps.hdop.isValid())
    // {
    //     second_line = "HD:" + String(gps.hdop.hdop(), 1);
    // }
    // else
    // {
    //     second_line = "-";
    // }
    // if (gps.course.isValid())
    // {
    //     second_line = second_line + " H:" + String((int)gps.course.deg());
    // }
    // else
    // {
    //     second_line = second_line + " -";
    // }
    // if (gps.speed.isValid())
    // {
    //     second_line = second_line + " S:" + String(gps.speed.knots(), 1);
    // }
    // else
    // {
    //     second_line = second_line + " -";
    // }
    // display.drawString(64, 26, second_line);
    // String third_line = String("Rudder: ");
    // third_line = third_line + String(rudder_control, 0);
    // display.drawString(64, 36, third_line);
    // display.display();
}

void autopilot::update_gps()
{
    while (Serial.available() > 0)
    {
        gps.encode(Serial.read());
        yield();
    }

    if (millis() > 5000 && gps.charsProcessed() < 10)
    {
       // state = 0;
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
    //rudder_control = 0;
    //setting state to autopilot
    if (new_state == 3)
    {
        //change this back!
        if (gps.location.isValid() && gps.course.isValid())
        {
            waypoint_lat = gps.location.lat();
            waypoint_long = gps.location.lng();
            heading = gps.course.deg();
            state = 3;
            rudder_engaged = true;
            return 1;
        }
        else
        {
            //do not have GPS lock; could not enable autopilot
            state = 1;
            rudder_engaged = false;
            return 0;
        }
    }
    //setting state to manual control mode
    if (new_state == 2)
    {
        state = 2;
        rudder_engaged = true;
        return 1;
    }
    //setting state to OFFLINE
    if (new_state == 1)
    {
        state = 1;
        rudder_engaged = false;
        return 1;
    }
    rudder_engaged = false;
    return 0;
}

int autopilot::set_heading(String command)
{
    int new_heading = command.toInt();
    if (new_heading >= 0 && new_heading <= 360 && gps.location.isValid() && gps.course.isValid())
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
    last_manual_message = millis();
    if (manual_command >= -100 && manual_command <= 100 && state == 2)
    {
        rudder_control = manual_command;
        return 1;
    }
    return 0;
}

int autopilot::pplus(String command)
{
    p_gain = p_gain + 0.25;
    return 1;
}
int autopilot::pminus(String command)
{
    p_gain = p_gain - 0.25;
    return 1;
}

int autopilot::dplus(String command)
{
    d_gain = d_gain + 0.25;
    return 1;
}
int autopilot::dminus(String command)
{
    d_gain = d_gain - 0.25;
    return 1;
}

int autopilot::ddplus(String command)
{
    dd_gain = dd_gain + 0.25;
    return 1;
}
int autopilot::ddminus(String command)
{
    dd_gain = dd_gain - 0.25;
    return 1;
}

void autopilot::pid_control()
{
    //autopilot mode
    if (state == 3)
    {
        //control via PID
        if (gps.location.isValid() && gps.course.isValid())
        {
            //have GPS lock, proceed to autopilot
            calculate_cross_track_error();
            calculate_direction_error();
            float p_error = p_gain * cross_track_error;
            float d_error = d_gain * direction_error;
            float dd_control = dd_gain * dd_error;
            //limit boat from spinning in circles if you are too far away from waypoint
            if (p_error > 50)
                p_error = 50;
            if (p_error < -50)
                p_error = -50;

            if (dd_control > 50)
                dd_control = 50;
            if (dd_control < -50)
                dd_control = -50;

            rudder_control = (p_error + d_error + dd_control);
            if (rudder_control > 100)
                rudder_control = 100;
            if (rudder_control < -100)
                rudder_control = -100;
        }
        else
        {
            rudder_control = 0;
        }
    }
    else if (state == 2)
    {
        //control rudder speed manually
        //check for timeout here
        if (millis() - last_manual_message > 4000)
        {
            rudder_control = 0;
        }
    }
    else
    {
        //do not move rudder at all
        rudder_control = 0;
    }
    //update motor control here
    //run_motor();
}

/*
 *  Calculates and saves cross-track error (units are meters)
 *  to the right of the line: positive
 *  to the left of the line: negative
 */
void autopilot::calculate_cross_track_error()
{
    float last_cross_track = cross_track_error;

    float theta12 = heading * (PI / 180.0);
    float theta13 = TinyGPSPlus::courseTo(waypoint_lat, waypoint_long, lat, lng) * (PI / 180);
    float d13 = TinyGPSPlus::distanceBetween(waypoint_lat, waypoint_long, lat, lng) / radius;
    cross_track_error = asin(sin(d13) * sin(theta13 - theta12)) * radius;
    if (cross_track_error != last_cross_track){
        dd_error = (cross_track_error - last_cross_track) * 1000 / ((millis() - last_error_calculation));
        last_error_calculation = millis();
    }
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

void autopilot::run_motor()
{
    //polled to control motor
    //rudder_control is between -100 and 100
    //read sensor and control motor accordingly
    // int raw_reading = 0;
    // int m = 5;
    // for (int i = 0; i < m; i++)
    // {
    //     raw_reading = raw_reading + analogRead(SENSOR);
    // }
    int raw_reading = analogRead(SENSOR);

    delay(5);
    float scaled_reading = map(raw_reading, 1024, 0, -100, 100);
    rudder_position = scaled_reading;
    //above 100 = right
    //less than 100 = left

    if (scaled_reading - rudder_control < -2)
    {
        //move rudder left
        digitalWrite(RIGHT, LOW);
        analogWrite(LEFT, limit(abs(scaled_reading - rudder_control) * 200,1024));
       // digitalWrite(LEFT, HIGH);
    }
    else if (scaled_reading - rudder_control > 2)
    {
        //move rudder right
        digitalWrite(LEFT, LOW);
        analogWrite(RIGHT, limit(abs(scaled_reading - rudder_control) * 200,1024));
    }
    else
    {
        //disable motor, in the deadzone
        digitalWrite(LEFT, LOW);
        digitalWrite(RIGHT, LOW);
    }
}

float autopilot::limit(float value, float limit) {
    if (value > limit){
        return limit;
    }
    else if (value < -limit){
        return -1 * limit;
    }
    else {
        return value;
    }

}