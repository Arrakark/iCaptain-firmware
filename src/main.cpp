#include <Arduino.h>
// do this before including other stuff like wificlient.h

#define PWMRANGE 1023

#include <Autopilot.h>
#include <ESP8266WiFi.h>
#include <aREST.h>

aREST rest = aREST();

const char *ssid = "autopilot";
const char *password = "flipflops";

#define LISTEN_PORT 80
WiFiServer server(LISTEN_PORT);
IPAddress local_IP(192, 168, 0, 1);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiClient client;

void clientHandle()
{
    client = server.available();
    //client.setNoDelay(1);
    while (client.connected() || client.available())
    {  
        if (client.available())
        {
            rest.handle(client);
        }
        // autopilot::run_motor();
        yield();
    }
}

void setup()
{
    //ESP.eraseConfig();
    
    Serial.begin(9600);
    Serial.swap();
    
    //pinMode(EN,OUTPUT);
    pinMode(LEFT,OUTPUT);
    pinMode(RIGHT,OUTPUT);
    pinMode(SENSOR,INPUT);
    
    digitalWrite(RIGHT, LOW);
    digitalWrite(LEFT, LOW);
    
    autopilot::init();

    // variable to be exposed
    rest.variable("state", &autopilot::state);
    rest.variable("heading", &autopilot::heading);
    //rest.variable("waypoint_long", &autopilot::waypoint_long);
    //rest.variable("waypoint_lat", &autopilot::waypoint_lat);
    //rest.variable("lat", &autopilot::lat);
    //rest.variable("long", &autopilot::lng);
    //rest.variable("hdop", &autopilot::hdop);
    rest.variable("course", &autopilot::course);
    //rest.variable("speed", &autopilot::speed);
    //rest.variable("sat_count", &autopilot::sat_count);
    //rest.variable("rudder_control", &autopilot::rudder_control);
    rest.variable("rudder_position", &autopilot::rudder_position);
    rest.variable("cross_track_error", &autopilot::cross_track_error);
    rest.variable("direction_error", &autopilot::direction_error);
    rest.variable("p_gain", &autopilot::p_gain);
    rest.variable("d_gain", &autopilot::d_gain);
    rest.variable("dd_gain", &autopilot::dd_gain);
    rest.variable("dd_error", &autopilot::dd_error);

    // //function to be exposed
    rest.function("set_heading", autopilot::set_heading);
    rest.function("set_state", autopilot::set_state);
    rest.function("manual_control", autopilot::manual_control);
    rest.function("pplus", autopilot::pplus);
    rest.function("pminus", autopilot::pminus);
    rest.function("dplus", autopilot::dplus);
    rest.function("dminus", autopilot::dminus);
    rest.function("ddplus", autopilot::ddplus);
    rest.function("ddminus", autopilot::ddminus);


    //rest.set_id("1");
    //rest.set_name("autopilot");


    ESP.eraseConfig();
    WiFi.softAPConfig(local_IP, gateway, subnet);
    //WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    server.begin();
}

void loop()
{
    //run autopilot
    //autopilot::update_display(); //once a second
    autopilot::update_values();  //five times a second
    autopilot::pid_control();    //as frequently as possible
    autopilot::update_gps();     //as frequently as possible
    //Serial.println(ESP.getFreeHeap());
    autopilot::run_motor();
    clientHandle();
}
