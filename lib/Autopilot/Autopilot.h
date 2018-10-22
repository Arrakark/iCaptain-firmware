#pragma once

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include <SSD1306.h> // alias for `#include "SSD1306Wire.h"`
#include <TinyGPS++.h>
#include <CmdMessenger.h>  // CmdMessenger

#define MENU_BUTTON D4
#define DOWN_BUTTON D0
#define UP_BUTTON 11

class autopilot
{
  public:
    autopilot();
    void init();
    void update_display();
    void update_gps();
    int state; //0 = error; 1 = off; 2 = manual rudder; 3 = autopilot
    int rudder_manual;
    int heading;
    float waypoint_long, waypoint_lat;
  private:
    void set_state(int);
    void set_heading(int);
    void set_rudder(int);
};