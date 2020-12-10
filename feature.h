#ifndef _FEATURE_H_
#define _FEATURE_H_

#include <Arduino.h>
#include "driver/gpio.h"
#include <SmartMatrix3.h>
#include "colorwheel.c"
#include "gimpbitmap.h"
#include <MPU6050_tockn.h>
#include <forcedClimate.h> // Lib changed to SDA17, SCL 16 !
#include <WiFi.h>
#include <ezTime.h>
#include "MMA7660.h"

#define Nr_sandkorns    500 // Number of sandkorns of sand
#define WIDTH           64 // Display width in pixels
#define HEIGHT          64 // Display height in pixels
#define MAX_X           ((WIDTH  * 256) - 1) // Maximum X coordinate in sandkorn space
#define MAX_Y           ((HEIGHT * 256) - 1) // Maximum Y coordinate
#define NUM_COLOURS     8
#define Debug           0
#define More_Random     1
#define COLOR_DEPTH     24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
#define GPIO_BIT_MASK  (1ULL<<GPIO_NUM_13)
#define Resetbutton     13
#define BME280          0
#define MPU6050         0 // 0= MMA7660


#include <WebServer.h>
#include <ESPmDNS.h>
#include <WebConfig.h>

String params = "["
  "{"
  "'name':'ssid',"
  "'label':'Name des WLAN',"
  "'type':"+String(INPUTTEXT)+","
  "'default':''"
  "},"
  "{"
  "'name':'pwd',"
  "'label':'WLAN Passwort',"
  "'type':"+String(INPUTPASSWORD)+","
  "'default':''"
  "},"
  "{"
  "'name':'letters',"
  "'label':'2Buchstaben',"
  "'type':"+String(INPUTTEXT)+","
  "'default':'Hi'"
  "}"
  "]";



// Set a timezone using the following list
// https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
#define MYTIMEZONE "Europe/Berlin"

//bool secondChanged()
//bool minuteChanged()

extern void SetSandkorn( int sandcnt, int xx, int  yy);



struct Sandkorn_t {
  int16_t  x,  y; // Position
  int16_t vx, vy; // Velocity
  uint16_t pos;
  rgb24 colour;
};

extern struct   Sandkorn_t sandkorn[1023];
extern uint16_t img[WIDTH * HEIGHT]; // Internal 'map' of pixels

#endif
