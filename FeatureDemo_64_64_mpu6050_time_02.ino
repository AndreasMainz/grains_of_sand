/*
    SmartMatrix Features Demo - Louis Beaudoin (Pixelmatix)
    This example code is released into the public domain
*/

#include "feature.h"

WebServer server;
WebConfig conf;

// Generate sensor object
// for Acceleration

#if MPU6050
MPU6050 mpu(Wire);
#else
MMA7660 mpu2;
#endif
#if BME280
// for weather data
ForcedClimate Bme280 = ForcedClimate(Wire, 0x76);
#endif

////////////// add RAM for Sandkorn stuff ////////////////////////

struct    Sandkorn_t sandkorn[1023];
uint16_t  img[WIDTH * HEIGHT]; // Internal 'map' of pixels
float     Pressure, Temperature, Humidity;
char      TemperatureString[6];
char      HumidityString[7];
char      PressureString[7];
uint16_t  Isr_tone_duration, Isr_tone_value;
hw_timer_t *timer2 = NULL;
String      letters;
char buchstaben[3];
boolean   Wifi_connected;
int8_t    x,y,z;




uint16_t CLOCK;
uint16_t AdcValue;
const int Beep_out = 15;
uint16_t NrOfSandkorns;

rgb24 myCOLORS[8] = {{0xFF, 0xFF, 0x00}, {0xFF, 0xFF, 0xFF}, {0xff, 0x00, 0x00}, {0x00, 0xFF, 0x00}, {0x00, 0x00, 0xFF}, {0x00, 0xFF, 0x00}, {0xFF, 0xFF, 0x00}, {0xFF, 0xFF, 0xFF}};
// rgb24 myCOLORS[8]={{0x00, 0x00, 0xFF},{0xff, 0x00, 0x00},{0x00, 0xff, 0x00},{0xff, 0xff, 0x00},{0xff, 0x00, 0xff},{0x00, 0xff, 0xff},{0xff, 0x30, 0x00},{0xff, 0xff, 0xFF}};

float xOffset, yOffset;
const uint8_t kMatrixWidth  = 64;       // known working: 32, 64, 96, 128
const uint8_t kMatrixHeight = 64;       // known working: 16, 32, 48, 64
const uint8_t kRefreshDepth = 24;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 2;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kPanelType =               SMARTMATRIX_HUB75_64ROW_MOD32SCAN;   // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
const uint8_t kMatrixOptions =          (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
const uint8_t kScrollingLayerOptions =  (SM_SCROLLING_OPTIONS_NONE);
const uint8_t kIndexedLayerOptions =    (SM_INDEXED_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);
//SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);

const int defaultBrightness = 255;        // full (100%) brightness
const rgb24 defaultBackgroundColor = {0x00, 0x00, 0x00};
Timezone myTZ;
unsigned long prevEpoch;
uint8_t Pos0, Pos1, Pos2, Pos3, Pos4, Pos5;
uint8_t Fertig0, Fertig1, Fertig2, Fertig3, Fertig4, Fertig5;


#include "Digit.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SetSandkorn( int sandcnt, int xx, int yy) {
  int imgIndex = 0;
  sandkorn[sandcnt].x = ((xx)  * 256);
  sandkorn[sandcnt].y = ((yy)  * 256);
  //sandkorn[sandcnt].vx = sandkorn[sandcnt].vy = 0; // Initial velocity is zero
  sandkorn[sandcnt].colour = (rgb24)myCOLORS[2]; // %NUM_COLOURS];
  sandkorn[sandcnt].pos = (sandkorn[sandcnt].y / 256) * WIDTH + (sandkorn[sandcnt].x / 256);
  imgIndex = sandkorn[sandcnt].pos;
  img[imgIndex] = 255;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void IRAM_ATTR ISR_tone()
{
  Isr_tone_duration++;
  if (Isr_tone_duration < 10)
  {
    Isr_tone_value = !Isr_tone_value; //toggle
    digitalWrite(Beep_out, Isr_tone_value);
  }
  else
  {
    timerAlarmDisable(timer2);
    Isr_tone_duration = 0;
  }
}
////////////////////////////////////////////////////////

boolean initWiFi() {
    boolean connected = false;
    boolean req_reset = false;
    WiFi.mode(WIFI_STA);
    Serial.print("Verbindung zum Wlan Netzwerk: ");
    Serial.print(conf.values[0]);
    Serial.println(" herstellen");
    req_reset = digitalRead(Resetbutton)?0:1;
    Serial.print("req_reset: ");Serial.println(req_reset);
    // if ((conf.values[0] != "")&&0) {
    if (conf.values[0] != "") {      
      WiFi.begin(conf.values[0].c_str(),conf.values[1].c_str());
      uint8_t cnt = 0;
      Serial.println(letters);
      while ((WiFi.status() != WL_CONNECTED) && (cnt<16)){
        delay(400);
        Serial.print(".");
        if (req_reset)break;
        cnt++;
      }
      Serial.println();
      if (WiFi.status() == WL_CONNECTED) {
        Serial.print("WLan connect erfolgreich IP-Adresse = ");
        Serial.println(WiFi.localIP());
        connected = true;
      }
    }
    Serial.print("Connected:");Serial.println(connected);
    if (!connected) {
          WiFi.mode(WIFI_AP);
          WiFi.softAP("Sanduhr","",1);  
          Serial.print("Starte AP !");
    }
    return connected;
}

void handleRoot() {
  conf.handleFormRequest(&server);
  if (server.hasArg("SAVE")) {
    uint8_t cnt = conf.getCount();
    Serial.println("*********** Konfiguration ************");
    for (uint8_t i = 0; i<cnt; i++) {
      Serial.print(conf.getName(i));
      Serial.print(" = ");
      Serial.println(conf.values[i]);
    }
    if (conf.getBool("switch")) Serial.printf("%s %s %i %5.2f \n",
                                conf.getValue("ssid"),
                                conf.getString("continent").c_str(), 
                                conf.getInt("amount"), 
                                conf.getFloat("float"));
  }
}

///////////////////////////////////////////////////////
void setup() {
  gpio_config_t io_conf;
  
  Serial.begin(115200);  
   
  // io_conf.intr_type = GPIO_PIN_INTR_DISABLE;//disable interrupt
//  io_conf.mode = GPIO_MODE_INPUT;//set as inputmode
//  io_conf.pin_bit_mask = (1ULL<<13);//bit mask of the pins that you want to set,e.g.GPIO15
//  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;//disable pull-down mode
//  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;//enable pull-up mode
//  gpio_config(&io_conf);//configure GPIO with the given settings

  // initialize the digital pin as an output.
  Serial.println("mpu6050 setup");
  Wire.begin(17, 16); //SDA / SCL
  #if MPU6050
  mpu.begin();
  Serial.println("mpu6050 setup done");
  xOffset = mpu.getRawAccX();
  yOffset = mpu.getRawAccY();
  Serial.print(xOffset);
  Serial.print(" | ");
  Serial.println(yOffset);
  #else
  mpu2.init();  
  mpu2.getXYZ(&x,&y,&z);
  xOffset = x;
  yOffset = y;
  #endif
  // mpu.calcGyroOffsets(true,0,0);


  ///////////////////////////////////////////////////////////
  #if BME280
  Bme280.begin();
  #endif
  /////////////////////////////////////////////////////////////
  // Read Adc for number of sandkörner
  NrOfSandkorns = Nr_sandkorns;
  AdcValue = analogRead(39) / 4;
  if (AdcValue)
  {
    NrOfSandkorns = AdcValue;
  }
  Serial.print("Adc: "); Serial.println(AdcValue);
  ////////////////////////////////////////////////////////////
//  pinMode(Beep_out, OUTPUT);
//  timer2 = timerBegin(1, 80, true);
//  /* Attach onTimer function to our timer */
//  timerAttachInterrupt(timer2, &ISR_tone, true);
//  timerAlarmWrite(timer2, 10000, true); // alle 10 ms

//  pinMode(Resetbutton, INPUT_PULLUP);


  Serial.println("backgroundLayer setup");
  matrix.addLayer(&backgroundLayer);
  Serial.println("backgroundLayer setup End");
  //matrix.addLayer(&scrollingLayer);
  matrix.addLayer(&indexedLayer);
  Serial.println("Matrix Begin");
  matrix.begin();
  Serial.println("Matrix Begin End");

  matrix.setBrightness(defaultBrightness);
  backgroundLayer.enableColorCorrection(false);
  backgroundLayer.setFont(font8x13);

  int i, j, bytes;
  for (i = 0; i < NrOfSandkorns; i++) { // For each sandkorn...

    int imgIndex = 0;
    do {
      sandkorn[i].x = random(WIDTH  * 256); // Assign random position within
      sandkorn[i].y = random(HEIGHT * 256); // the 'sandkorn' coordinate space
      // Check if corresponding pixel position is already occupied...
      for (j = 0; (j < i) && (((sandkorn[i].x / 256) != (sandkorn[j].x / 256)) ||
                              ((sandkorn[i].y / 256) != (sandkorn[j].y / 256))); j++);
      imgIndex = (sandkorn[i].y / 256) * WIDTH + (sandkorn[i].x / 256);
    } while (img[imgIndex] != 0); // Keep retrying until a clear spot is found
    img[imgIndex] = 255; // Mark it
    sandkorn[i].pos = (sandkorn[i].y / 256) * WIDTH + (sandkorn[i].x / 256);
    sandkorn[i].vx = sandkorn[i].vy = 0; // Initial velocity is zero
    sandkorn[i].colour = myCOLORS[i & 0x7]; // %NUM_COLOURS];
  }
  // Serial.println("Setup done");
  indexedLayer.setFont(font6x10);
  indexedLayer.setIndexedColor(1, {0xff, 0xff, 0xff}); //weiß

  conf.setDescription(params);
  conf.readConfig();

  if (Wifi_connected = initWiFi())
  {
    Serial.println("initWiFi done");
  }
  else
  {
    Serial.println("Wlan connect failed");
  }
  char dns[30];
  sprintf(dns,"%s.local","Sanduhr");
  Serial.println(dns);
  if (MDNS.begin(dns)) {
    Serial.println("MDNS responder gestartet");
  }
  server.on("/",handleRoot);
  Serial.println("Server on done");
  server.begin(80);
  Serial.println("Server begin done");
  
  // timerAlarmEnable(timer2);
  if (WiFi.status() == WL_CONNECTED)
  {
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  myTZ.setLocation(MYTIMEZONE);
  waitForSync();
  }

}

///////////////////////////////////////////////////////////////
// the loop() method runs over and over again,
// as long as the board has power


void loop() {
  CLOCK++;
  server.handleClient();
  // Delay(50);
  ///////////
  uint16_t stunden, minuten, sekunden;
  unsigned long epoch;
  // Get time in format "01:15" or "22:15"(24 hour with leading 0)
  stunden = myTZ.dateTime("H").toInt();
  minuten = myTZ.dateTime("i").toInt();
  sekunden = myTZ.dateTime("s").toInt();
#if DEBUG
  Serial.print(stunden);
  Serial.println("time ");
  Serial.print(sekunden);
#endif

  ////////////
  int i, j, value, current_pos;
  
  backgroundLayer.fillScreen(defaultBackgroundColor);
  if (Wifi_connected)
  {
    /////////////////////////////////////////////////////////////////////////
    backgroundLayer.Digit0();
    if (((sekunden % 10) != Pos0) || ((Fertig0 > 0) && (Fertig0 < 5)))
    {
      Fertig0 += 1;
      Pos0 = sekunden % 10;
      backgroundLayer.morphNumber(Pos0, Fertig0);
    }
    else
    {
      Fertig0 = 0;
      backgroundLayer.drawNumber(sekunden % 10);
    }
    /////////////////////////////////////////////////////////////////////////
    
    backgroundLayer.Digit1();
    // Male Doppelpunkte
    backgroundLayer.fillRectangle(41, 6,  42, 7, {255,255,0});
    backgroundLayer.fillRectangle(41, 11, 42, 12,{255,255,0});
    if (Fertig0 == 0) {
      // Morphing wäre notwendig
      if (((sekunden / 10) != Pos1) || (Fertig1 > 0 && Fertig1 < 5))
      {
        Pos1 = sekunden / 10;
        backgroundLayer.morphNumber(Pos1,Fertig1);
        Fertig1 ++;
      }
      else
      {
        Fertig1 = 0;
        backgroundLayer.drawNumber(Pos1);
        //backgroundLayer.drawNumber(sekunden % 10);
      }
    }
    else
    { // Male alte Zahl weiter
      Fertig1 = 0;
      backgroundLayer.drawNumber(Pos1);
    }
    ///////////////////////////////////////////////////////////////////////////
    backgroundLayer.Digit2();
    if ((!Fertig1)&&(!Fertig0)) {
      // Morphing wäre notwendig
      if (((minuten % 10) != Pos2) || (Fertig2 > 0 && Fertig2 < 5))
      {
        Pos2 = minuten % 10;
        backgroundLayer.morphNumber(Pos2, Fertig2);
        Fertig2 ++;
      }
      else
      {
        Fertig2 = 0;
        backgroundLayer.drawNumber(Pos2);
      }
    }
    else
    { // Male alte Zahl weiter
      backgroundLayer.drawNumber(Pos2);
    }
    ////////////////////////////////////////////////////////////////////////////////
    backgroundLayer.Digit3();
    // Male Doppelpunkte
    backgroundLayer.fillRectangle(20, 6,  21, 7, {255,255,255});
    backgroundLayer.fillRectangle(20, 11, 21, 12,{255,255,255});
  
    if ((!Fertig1)&&(!Fertig0)&&(!Fertig2)) {
      // Morphing wäre notwendig
      if (((minuten / 10) != Pos3) || (Fertig3 > 0 && Fertig3 < 5))
      {
        Pos3 = minuten / 10;
        backgroundLayer.morphNumber(Pos3,Fertig3);
        Fertig3 ++;
      }
      else
      {
        Fertig3 = 0;
        backgroundLayer.drawNumber(Pos3);
      }
    }
    else
    { // Male alte Zahl weiter
      backgroundLayer.drawNumber(Pos3);
    }
  
    backgroundLayer.Digit4();
    if ((!Fertig1)&&(!Fertig0)&&(!Fertig2)&&(!Fertig3)) {
      // Morphing wäre notwendig
      if (((stunden % 10) != Pos4) || (Fertig4 > 0 && Fertig4 < 5))
      {
        Pos4 = stunden % 10;
        backgroundLayer.morphNumber(Pos4,Fertig4);
        Fertig4 ++;
      }
      else
      {
        Fertig4 = 0;
        backgroundLayer.drawNumber(Pos4);
      }
    }
    else
    { // Male alte Zahl weiter
      backgroundLayer.drawNumber(Pos4);
    }
    //////////////////////////////////////////////////////////////////////
    backgroundLayer.Digit5();
    // backgroundLayer.drawNumber(stunden / 10);
      if ((!Fertig1)&&(!Fertig0)&&(!Fertig2)&&(!Fertig3)&&(!Fertig4)) {
      // Morphing wäre notwendig
      if (((stunden / 10) != Pos5) || (Fertig5 > 0 && Fertig5 < 5))
      {
        Pos5 = stunden / 10;
        backgroundLayer.morphNumber(Pos5,Fertig5);
        Fertig5 ++;
      }
      else
      {
        Fertig5 = 0;
        backgroundLayer.drawNumber(Pos5);
      }
    }
    else
    { // Male alte Zahl weiter
      backgroundLayer.drawNumber(Pos5);
    }
  }
  

  if ((CLOCK > 0x0ff)) {
    // if ((CLOCK && 0x0F)== 0)
    // backgroundLayer.morphNumber(2);
  } else backgroundLayer.drawNumber(1);

  ///////////////////////////////////////////////////////
#if BME280
  if ((CLOCK & 0xFFF) == 0) 
  {
    Pressure    = Bme280.getPressure(true);
    Temperature = Bme280.getTemperatureCelcius(true);
    Humidity    = Bme280.getRelativeHumidity(true);
    dtostrf(Humidity,    5, 2, HumidityString);
    dtostrf(Pressure,    6, 2, PressureString);
    dtostrf(Temperature, 6, 2, TemperatureString);

    indexedLayer.drawString(1, 1, 1, TemperatureString);
    //indexedLayer.drawString(1,1,1,strcat("T ",TemperatureString));
    //indexedLayer.swapBuffers();
    //indexedLayer.setIndexedColor(1,{0x00, 0x00, 0xff}); // blau
    indexedLayer.drawString(1, 11, 1, HumidityString);
    //indexedLayer.swapBuffers();
    //indexedLayer.setIndexedColor(1,{0x00, 0xff, 0x00}); // grün
    indexedLayer.drawString(1, 21, 1, PressureString);
    indexedLayer.swapBuffers(false);
#if Debug
    Serial.print(Pressure);
    Serial.print(" | ");
    Serial.print(Humidity);
    Serial.print(" | ");
    Serial.println(Temperature);
#endif

  }
  else
  {
    mpu.update(); // Modified in Lib!! Liest nur Rohwerte !
  }
 #else
   #if MPU6050
     mpu.update(); // Modified in Lib!! Liest nur Rohwerte !
   #endif
#endif //#if BME280
  //////////////
  //////////////

//toCharArray(buf, len)
  ///////// Text Anzeige ///////////////////////////////
  // Werte aus Eeprom laden
  letters = conf.getletters();
  // Serial.print(letters);
  //Konvertieren zu Char array
  letters.toCharArray(buchstaben,3);
  // Ausgabe auf Display Sand sensitive
  backgroundLayer.drawString(9, 10, {0xff, 0, 0},buchstaben , 2, 2);
  // backgroundLayer.drawString(9, 10, {0xff, 0, 0},"Hi", 2, 2);
  /////////////////////////////////////////////////////////
  // Read accelerometer...
  #if MPU6050
  float accelX = (mpu.getRawAccX() * -1) + xOffset;
  float accelY = (mpu.getRawAccY() * -1) + yOffset;
  float accelZ = (mpu.getRawAccZ() * -1);
  #else
  mpu2.getXYZ(&x,&y,&z);
  float accelX = (x * -1) + xOffset;
  float accelY = (y * -1) + yOffset;
  float accelZ = (z * -1);
  #endif
  int16_t az2;
          if ((sekunden / 20) & 1){
            az2 = sekunden % 20;
          }
          else
          {
            az2 = 20 - (sekunden % 20);
          }
          az2 -= 10;
#if MPU6050
  int16_t ax = (-1) * accelY/ 256,     // Transform accelerometer axes
          ay = (1) * accelZ/ 1024,      // to sandkorn coordinate space
          az = abs(accelX) / 2048; // Random motion factor
          ax -= 4;          
          if (ax == 0) ax++;
          ax *= 3;
          ax *= az2;
          if (ay == 0) ay++;
          // ay *= 3;
          ay = ay * (-az2);
#else
int16_t ax = (-1) * accelY,     // Transform accelerometer axes
          ay = (1) * accelZ,      // to sandkorn coordinate space
          az = abs(accelX); // Random motion factor
#endif
          #if Debug
          Serial.print(az2);
          Serial.print(" | ");
          #endif
          
#if 0         
          if ((sekunden / 10) & 1){
          //ax += ((sekunden/16)*16);
          ay += (sekunden);
          }
          else
          {
          //ax -= ((sekunden/16)*16);
          ay -= (sekunden);
            
          }
#endif
  //az = (az >= 4) ? 1 : 4 - az;      // Clip & invert
  az = (az >= 8) ? az : 8 - az;      // Clip & invert
#if More_Random
  ax -= az;                         // Subtract motion factor from X, Y
  ay -= az;
  az2 = az * 8 + 1;         // Range of random motion to add back in
#else
  az2 = az * 8 + 1;         // Range of random motion is zero
#endif
  if (ax == (-1)) ax = 0;
  if (ax == (1))  ax = 0;
  if (ay == (-1)) ay = 0;
  if (ay == (1))  ay = 0;


#if Debug || 1
  Serial.print(ax);
  Serial.print(" | ");
  Serial.print(ay);
  Serial.print(" | ");
  Serial.println(az);
#endif
  // ...and apply 2D accel vector to sandkorn velocities...
  int32_t v2; // Velocity squared
  float   v;  // Absolute velocity
  for (int i = 0; i < NrOfSandkorns; i++) {
    sandkorn[i].vx += ax + random(az2); // A little randomness makes
    sandkorn[i].vy += ay + random(az2); // tall stacks topple better!
    // Terminal velocity (in any direction) is 256 units -- equal to
    // 1 pixel -- which keeps moving sandkorns from passing through each other
    // and other such mayhem.  Though it takes some extra math, velocity is
    // clipped as a 2D vector (not separately-limited X & Y) so that
    // diagonal movement isn't faster
    v2 = (int32_t)sandkorn[i].vx * sandkorn[i].vx + (int32_t)sandkorn[i].vy * sandkorn[i].vy;
    if (v2 > 65536) { // If v^2 > 65536, then v > 256
      v = sqrt((float)v2); // Velocity vector magnitude
      sandkorn[i].vx = (int)(256.0 * (float)sandkorn[i].vx / v); // Maintain heading
      sandkorn[i].vy = (int)(256.0 * (float)sandkorn[i].vy / v); // Limit magnitude
    }
  }

  // ...then update position of each sandkorn, one at a time, checking for
  // collisions and having them react.  This really seems like it shouldn't
  // work, as only one sandkorn is considered at a time while the rest are
  // regarded as stationary.  Yet this naive algorithm, taking many not-
  // technically-quite-correct steps, and repeated quickly enough,
  // visually integrates into something that somewhat resembles physics.
  // (I'd initially tried implementing this as a bunch of concurrent and
  // "realistic" elastic collisions among circular sandkorns, but the
  // calculations and volument of code quickly got out of hand for both
  // the tiny 8-bit AVR microcontroller and my tiny dinosaur brain.)

  uint16_t       bytes, oldidx, newidx, delta;
  int16_t        newx, newy;

  for (i = 0; i < NrOfSandkorns; i++) {
    newx = sandkorn[i].x + sandkorn[i].vx; // New position in sandkorn space
    newy = sandkorn[i].y + sandkorn[i].vy;
    if (newx > MAX_X) {              // If sandkorn would go out of bounds
      newx         = MAX_X;          // keep it inside, and
      sandkorn[i].vx /= -2;          // give a slight bounce off the wall
      //timerAlarmEnable(timer2);
    } else if (newx < 0) {
      newx         = 0;
      sandkorn[i].vx /= -2;
      //timerAlarmEnable(timer2);
    }
    if (newy > MAX_Y) {
      newy = MAX_Y;
      sandkorn[i].vy /= -2;
      // timerAlarmEnable(timer2);
    }
    else if
    (newy < 0) {
      newy = 0;
      sandkorn[i].vy /= -2;
      // timerAlarmEnable(timer2);
    }

    oldidx = (sandkorn[i].y / 256) * WIDTH + (sandkorn[i].x / 256); // Prior pixel #
    newidx = (newy      / 256) * WIDTH + (newx      / 256); // New pixel #
    if ((oldidx != newidx) && // If sandkorn is moving to a new pixel...
        img[newidx]) {       // but if that pixel is already occupied...
      delta = abs(newidx - oldidx); // What direction when blocked?
      if (delta == 1) {           // 1 pixel left or right)
        newx         = sandkorn[i].x;  // Cancel X motion
        sandkorn[i].vx /= -2;          // and bounce X velocity (Y is OK)
        newidx       = oldidx;      // No pixel change
        // timerAlarmEnable(timer2);
      } else if (delta == WIDTH) { // 1 pixel up or down
        newy         = sandkorn[i].y;  // Cancel Y motion
        sandkorn[i].vy /= -2;          // and bounce Y velocity (X is OK)
        newidx       = oldidx;      // No pixel change
        //timerAlarmEnable(timer2);
      } else { // Diagonal intersection is more tricky...
        // Try skidding along just one axis of motion if possible (start w/
        // faster axis).  Because we've already established that diagonal
        // (both-axis) motion is occurring, moving on either axis alone WILL
        // change the pixel index, no need to check that again.
        if ((abs(sandkorn[i].vx) - abs(sandkorn[i].vy)) >= 0) { // X axis is faster
          newidx = (sandkorn[i].y / 256) * WIDTH + (newx / 256);
          if (!img[newidx]) { // That pixel's free!  Take it!  But...
            newy         = sandkorn[i].y; // Cancel Y motion
            sandkorn[i].vy /= -2;         // and bounce Y velocity
            // timerAlarmEnable(timer2);
          } else { // X pixel is taken, so try Y...
            newidx = (newy / 256) * WIDTH + (sandkorn[i].x / 256);
            if (!img[newidx]) { // Pixel is free, take it, but first...
              newx         = sandkorn[i].x; // Cancel X motion
              sandkorn[i].vx /= -2;         // and bounce X velocity
              // timerAlarmEnable(timer2);
            } else { // Both spots are occupied
              newx         = sandkorn[i].x; // Cancel X & Y motion
              newy         = sandkorn[i].y;
              sandkorn[i].vx /= -2;         // Bounce X & Y velocity
              sandkorn[i].vy /= -2;
              newidx       = oldidx;     // Not moving
            }
          }
        } else { // Y axis is faster, start there
          newidx = (newy / 256) * WIDTH + (sandkorn[i].x / 256);
          if (!img[newidx]) { // Pixel's free!  Take it!  But...
            newx         = sandkorn[i].x; // Cancel X motion
            sandkorn[i].vy /= -2;         // and bounce X velocity
            // timerAlarmEnable(timer2);
          } else { // Y pixel is taken, so try X...
            newidx = (sandkorn[i].y / 256) * WIDTH + (newx / 256);
            if (!img[newidx]) { // Pixel is free, take it, but first...
              newy         = sandkorn[i].y; // Cancel Y motion
              sandkorn[i].vy /= -2;         // and bounce Y velocity
              // timerAlarmEnable(timer2);
            } else { // Both spots are occupied
              newx         = sandkorn[i].x; // Cancel X & Y motion
              newy         = sandkorn[i].y;
              sandkorn[i].vx /= -2;         // Bounce X & Y velocity
              sandkorn[i].vy /= -2;
              newidx       = oldidx;     // Not moving
            }
          }
        }
      }
    }
    else
    {
      if ((abs(sandkorn[i].vx) > 2000)) {
        timerAlarmEnable(timer2);  //&&(newx > (MAX_X-1))
        Serial.println(newx);
      }
    }
    sandkorn[i].x  = newx; // Update sandkorn position
    sandkorn[i].y  = newy;
    img[oldidx] = 0;    // Clear old spot (might be same as new, that's OK)
    img[newidx] = 255;  // Set new spot
    sandkorn[i].pos = newidx;
    // Serial.println(newidx);
    /////////////////////////////////////////////// Ausgabe //////////////////////////
    backgroundLayer.drawPixel(sandkorn[i].x / 256, sandkorn[i].y / 256, sandkorn[i].colour);
  } // For Loop Sandkörner
  //////////////////////////////////////////////////////
  // backgroundLayer.swapBuffers(false);
  backgroundLayer.swapBuffers();

}
