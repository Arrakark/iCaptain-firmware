#include <Arduino.h>
// do this before including other stuff like wificlient.h
#include "lwip/tcp_impl.h"

void tcpCleanup()
{
    while (tcp_tw_pcbs != NULL)
    {
        tcp_abort(tcp_tw_pcbs);
    }
}

#include <Autopilot.h>
#include <ESP8266WiFi.h>
#include <aREST.h>

aREST rest = aREST();

const char *ssid = "autopilot";
const char *password = "flipflop";

#define LISTEN_PORT 80
WiFiServer server(LISTEN_PORT);
IPAddress local_IP(192, 168, 0, 1);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiClient client;

void clientHandle()
{
    tcpCleanup();
    client = server.available();
    //client.setNoDelay(1); experimental hack
    while (client.connected() || client.available())
    {  
        if (client.available())
        {
            rest.handle(client);
        }
        autopilot::run_motor();
        yield();
    }
}

void setup()
{
    ESP.eraseConfig();
    
    autopilot::init();

    // variable to be exposed
    rest.variable("state", &autopilot::state);
    rest.variable("heading", &autopilot::heading);
    //rest.variable("waypoint_long", &autopilot::waypoint_long);
    //rest.variable("waypoint_lat", &autopilot::waypoint_lat);
    rest.variable("lat", &autopilot::lat);
    rest.variable("long", &autopilot::lng);
    rest.variable("hdop", &autopilot::hdop);
    rest.variable("course", &autopilot::course);
    rest.variable("speed", &autopilot::speed);
    rest.variable("sat_count", &autopilot::sat_count);
    rest.variable("rudder_speed", &autopilot::rudder_speed);
    //rest.variable("cross_track_error", &autopilot::cross_track_error);
    //rest.variable("direction_error", &autopilot::direction_error);
    //rest.variable("p_gain", &autopilot::p_gain);
    //rest.variable("d_gain", &autopilot::d_gain);

    // //function to be exposed
    rest.function("set_heading", autopilot::set_heading);
    rest.function("set_state", autopilot::set_state);
    rest.function("manual_control", autopilot::manual_control);

    //rest.set_id("1");
    //rest.set_name("autopilot");

    // Setup WiFi and server
    WiFi.softAPConfig(local_IP, gateway, subnet);
    //WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    server.begin();
}

void loop()
{
    //run autopilot
    autopilot::update_display(); //once a second
    autopilot::update_values();  //five times a second
    autopilot::pid_control();    //as frequently as possible
    autopilot::update_gps();     //as frequently as possible
    autopilot::run_motor();
    yield();
    clientHandle();
}
