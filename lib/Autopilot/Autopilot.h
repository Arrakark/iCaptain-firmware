#pragma once

#include <Arduino.h>
// #include <Wire.h>    // Only needed for Arduino 1.6.5 and earlier
// #include <SSD1306.h> // alias for `#include "SSD1306Wire.h"`
#include <TinyGPS++.h>

//MOTOR DEFINE
#define LEFT D0
#define RIGHT D3
#define SENSOR A0

#define radius 6371008.8

class autopilot
{
public:
  static void init();
  static void update_display();
  static void update_gps();
  static void update_values();
  static void pid_control();
  static int state; //0 = error; 1 = off; 2 = manual rudder; 3 = autopilot
  static float heading; //which direction the boat is supposed to go
  static double waypoint_long, waypoint_lat; //location of last heading update
  static double lng, lat, hdop, course, speed;
  static double p_gain, d_gain;
  static int sat_count;
  static float rudder_control;
  static float rudder_position;
  static bool rudder_engaged;
  static long last_manual_message;
  static float cross_track_error;
  static float dd_error;
  static long last_error_calculation;
  static double dd_gain;
  static float direction_error;
  static int set_state(String);
  static int set_heading(String);
  static int manual_control(String);
  static int pplus(String);
  static int pminus(String);
  static int dplus(String);
  static int dminus(String);
  static int ddplus(String);
  static int ddminus(String);
  static void run_motor();
private:
  autopilot() {}
  static float limit(float, float);
  static void calculate_cross_track_error();
  static void calculate_direction_error();
};