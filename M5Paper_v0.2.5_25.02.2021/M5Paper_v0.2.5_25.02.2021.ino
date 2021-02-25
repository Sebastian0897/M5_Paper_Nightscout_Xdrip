#include <M5EPD.h>
#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Preferences.h>
#include <TimeLib.h>
#include <esp_wifi.h>
#include <esp_bt.h>
#define uS_TO_S_FACTOR 1000000
#define uS_TO_MS_FACTOR 1000
#define TIME_TO_SLEEP 800

M5EPD_Canvas canvas(&M5.EPD); // setup for the Display
Preferences preferences; // needed to save files permanently

// For the Clock
rtc_time_t RTCtime;
rtc_date_t RTCDate;
char timeStrbuff[64];

// For the Internet Connection
const char* ssid     = "ssid";
const char* password = "password";

const char* ssid1    = "ssid";
const char* password1= "password";

const char* ssid2    = "ssid";
const char* password2= "password";

const char* ssid3    = "ssid";
const char* password3= "password";

// For weather Display
String JsonBuffer1;
// Your Domain name with URL path or IP address with path
String openWeatherMapApiKey = "openWeatherMapApiKey";
// Replace with your country code and city
String city = "Berlin";
String countryCode = "DE";
String lang = "de"; 
String units = "metric"; // metric/imperial -> to get Celsius or Fahrenheit


// the following are required for the Bloodsugar Display
String JsonBuffer2;
//const char* url = "http://192.168.178.75:17580/pebble"; //Blood glukose local Xdrip API, seems to be unreliable. looking into it
String url = "https://NIGHTSCOUTSERVER/api/v1/entries.json?count=1"; //Blood glucose values from Nightscout.
String line; 
char updatetime[10];
int Zeit;
int GMTOffset = 1; // change if your RTC is not set to GMT time but to your local time
long  bz_time_alt;
long bz_time_alt_in_five;


void setup() {
    M5.begin(false,false,false,true,true); //Touchscreen,SDreader, Serial,BatteryADC,I2C
   Serial.println("Angeschaltet");
    M5.EPD.SetRotation(90); // for the Screen to be upright
    //M5.EPD.Clear(true);  //if you want a Full refresh after every start
    disableEXTPower();
 //   disableEPDPower();
}

void disableEXTPower() { digitalWrite(5, 0); }
void enableEXTPower()  { digitalWrite(5, 1); }
void enableEPDPower()  { digitalWrite(23, 1);}
void disableEPDPower() { digitalWrite(23, 0);}


void flushTime() // Update time from RTC
{   
    M5.RTC.begin();  // required for the Clock
    M5.RTC.getTime(&RTCtime);
    M5.RTC.getDate(&RTCDate);
   
sprintf(timeStrbuff,"%02d/%02d/%d %02d:%02d",
                        RTCDate.day,RTCDate.mon,RTCDate.year,
                        RTCtime.hour,RTCtime.min);
    canvas.drawString(timeStrbuff, 35, 5); // position of the time and date
}


void FullCleanDisplay() // Clean display from possible artifacts only for Debug
{
enableEPDPower();
canvas.deleteCanvas();  
canvas.createCanvas(540, 960); 
 canvas.pushCanvas(0,0,UPDATE_MODE_INIT); 
             delay(10);
}


void Angeschaltet() // To show that the device is currently working
{
enableEPDPower();
canvas.deleteCanvas();  
canvas.createCanvas(55, 30); // Size requiered for the Display
canvas.setTextSize(3); 
canvas.drawString("AN", 0, 0);  
canvas.pushCanvas(305,10,UPDATE_MODE_DU);  
}

void ConnectWifi() // Connect wifi. It tries the Network that you Configured at the beginning of the File first and than tries more networks you can configure here.
{  
   int i=0;
    while (WiFi.status() != WL_CONNECTED && i<3)// tries every network 3 times before moving on
    {
     WiFi.begin(ssid,password);
     delay(1000);
     if (WiFi.status() == WL_CONNECTED)
    {break; }
      WiFi.begin(ssid1,password1);
     delay(1000);
     if (WiFi.status() == WL_CONNECTED)
    {break; }
     WiFi.begin(ssid2,password2);
     delay(1000);
      if (WiFi.status() == WL_CONNECTED)
    {break; }
    WiFi.begin(ssid3,password3);
     delay(1000);
     if (WiFi.status() == WL_CONNECTED)
    {break; }
     i++;
     }
if (i>=3){  // if the loop ran 3 times no connection was established -> show no Connection
enableEPDPower();
canvas.deleteCanvas();  
canvas.createCanvas(290, 30); // Size requiered for the Weather
canvas.setTextSize(3); 
canvas.pushCanvas(10,10,UPDATE_MODE_DU4);   
canvas.drawString("NICHT Verbunden", 0, 0);  
canvas.pushCanvas(10,10,UPDATE_MODE_GC16);               
}
if(WiFi.status()== WL_CONNECTED){// show that it connected to a WIFI network
enableEPDPower();
canvas.deleteCanvas();  
canvas.createCanvas(290, 30); // Size requiered for the Weather
canvas.setTextSize(3); 
canvas.drawString("WLAN Verbunden", 0, 0);  
canvas.pushCanvas(10,10,UPDATE_MODE_DU); 
}
}

void UpdateWeather()  // Get weather info
{
ConnectWifi();
if(WiFi.status()== WL_CONNECTED){
      String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&lang="  + lang   + "&units=" + units + "&APPID=" + openWeatherMapApiKey;      
      JsonBuffer1 = httpGETRequest(serverPath.c_str());
WiFi.disconnect();
StaticJsonDocument<1024> doc;

DeserializationError error = deserializeJson(doc, JsonBuffer1);

if (error) {
  Serial.print(F("deserializeJson() failed: "));
  Serial.println(error.f_str());
  return;
}

float coord_lon = doc["coord"]["lon"]; // 13.4105
float coord_lat = doc["coord"]["lat"]; // 52.5244

JsonObject weather_0 = doc["weather"][0];
int weather_0_id = weather_0["id"]; // 800
const char* weather_0_main = weather_0["main"]; // "Clear"
const char* weather_0_description = weather_0["description"]; // "Klarer Himmel"
const char* weather_0_icon = weather_0["icon"]; // "01d"

const char* base = doc["base"]; // "stations"

JsonObject main = doc["main"];
float main_temp = main["temp"]; // -0.29
float main_feels_like = main["feels_like"]; // -5.45
float main_temp_min = main["temp_min"]; // -1.11
float main_temp_max = main["temp_max"]; // 0.56
int main_pressure = main["pressure"]; // 1033
int main_humidity = main["humidity"]; // 51

int visibility = doc["visibility"]; // 10000

float wind_speed = doc["wind"]["speed"]; // 3.09
int wind_deg = doc["wind"]["deg"]; // 140

int clouds_all = doc["clouds"]["all"]; // 0

long dt = doc["dt"]; // 1613390654

JsonObject sys = doc["sys"];
int sys_type = sys["type"]; // 1
int sys_id = sys["id"]; // 1262
const char* sys_country = sys["country"]; // "DE"
long sys_sunrise = sys["sunrise"]; // 1613370162
long sys_sunset = sys["sunset"]; // 1613405912

int timezone = doc["timezone"]; // 3600
long id = doc["id"]; // 2950159
const char* name = doc["name"]; // "Berlin"
int cod = doc["cod"]; // 200
if (main_pressure != 0){
canvas.deleteCanvas();  
canvas.createCanvas(400, 40); // Size requiered for the Weather
canvas.setTextSize(4); 
char temp[1];
dtostrf(main_temp,5,2,temp); // convert to the correct file format
canvas.drawString("Temperatur:"+String(temp), 0, 0);  
canvas.pushCanvas(10,100,UPDATE_MODE_GC16);
}
}
}

String httpGETRequest(const char* serverName) { // this is used to write the information from the webservices into a string. needed for weather and Nightscout
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  // Debug
  if (httpResponseCode>0) {
  //  Serial.print("HTTP Response code: ");
  //  Serial.println(httpResponseCode);
/*char code[10];                          // Debug if required
dtostrf(httpResponseCode,10,0,code);
canvas.deleteCanvas();  
canvas.createCanvas(540, 40); // 
canvas.setTextSize(5); 
canvas.drawString(code, 0, 0);  
canvas.pushCanvas(0,100,UPDATE_MODE_GC16);*/
    payload = http.getString();
  }
  else {
/*   Serial.print("Error code: ");
    Serial.println(httpResponseCode);
canvas.deleteCanvas();                     //Debug if required
canvas.createCanvas(540, 40); // Size requiered
canvas.setTextSize(5); 
canvas.drawString("Fehler", 0, 0);  
canvas.pushCanvas(0,100,UPDATE_MODE_GC16);  
  */
  }
  // Free resources
  http.end();
  return payload;
}

 
void BZdisplay() // Gets the current blood glucose from nightscout and displays it.
{
   M5.RTC.getTime(&RTCtime);
   M5.RTC.getDate(&RTCDate);
   
preferences.begin("BZ", false); //open or generate Folder BZ
//preferences.clear(); // only activate for Debug. it clears everything from the Openend Folder. in this case "BZ" 
bz_time_alt = preferences.getLong("bz_time", 0); //get the value from storage display 0 if empty
bz_time_alt_in_five = bz_time_alt + 305; // to run in 5 minutes add 300 secs and 5secs to account for small delays in the upload timing

setTime(RTCtime.hour,RTCtime.min,RTCtime.sec,RTCDate.day,RTCDate.mon,RTCDate.year);
time_t t = now();
//Serial.println(hour(t));
//Serial.println(minute(t));
//Serial.println(second(t));
Serial.print("Time now:");
Serial.println(now());
Serial.print("Time when to update:");
Serial.println(bz_time_alt_in_five);
if (bz_time_alt_in_five - now() <=30 && bz_time_alt_in_five - now() > 0 )
{
 Serial.println("going to sleep to wait for BZ");
 Serial.flush();
 esp_sleep_enable_timer_wakeup(400 * uS_TO_MS_FACTOR); // 400ms sleep for the EPD to finish updating otherwise there will be display artifacts
 gpio_hold_en((gpio_num_t) 2);// to prevent the main MOSfet from turning everything off
 gpio_deep_sleep_hold_en();
 esp_light_sleep_start();
 disableEPDPower();
 esp_sleep_enable_timer_wakeup((bz_time_alt_in_five - now()) * uS_TO_S_FACTOR); // Adjusts the time to sleep it self
 gpio_hold_en((gpio_num_t) 2);// to prevent the main MOSfet from turning everything off
 gpio_deep_sleep_hold_en();
 esp_light_sleep_start();
 Serial.println("woke up again");
 
}
if (bz_time_alt_in_five <= now() || RTCtime.min == 0  || RTCtime.min == 30 )
{
ConnectWifi();
delay(10);
if(WiFi.status()== WL_CONNECTED){    
 // create HTTP Client
      JsonBuffer2 = httpGETRequest(url.c_str());
    //  Serial.println(JsonBuffer2);
WiFi.disconnect();
StaticJsonDocument<512> doc;

DeserializationError error = deserializeJson(doc, JsonBuffer2);

if (error) {
  Serial.print(F("deserializeJson() failed: "));
  Serial.println(error.f_str());
  return;
}

JsonObject root_0 = doc[0];
const char* root_0__id = root_0["_id"]; // "6023c4e8106eeede8e691447"
const char* root_0_device = root_0["device"]; // "xDrip-LibreReceiver"
long long root_0_date = root_0["date"]; // 1612956904339
const char* root_0_dateString = root_0["dateString"]; // "2021-02-10T11:35:04.339Z"
int root_0_sgv = root_0["sgv"]; // 89
float root_0_delta = root_0["delta"]; // -0.996
const char* root_0_direction = root_0["direction"]; // "Flat"
const char* root_0_type = root_0["type"]; // "sgv"
long root_0_filtered = root_0["filtered"]; // 82000
long root_0_unfiltered = root_0["unfiltered"]; // 82000
int root_0_rssi = root_0["rssi"]; // 100
int root_0_noise = root_0["noise"]; // 1
const char* root_0_sysTime = root_0["sysTime"]; // "2021-02-10T11:35:04.339Z"
int root_0_utcOffset = root_0["utcOffset"]; // 60
long long root_0_mills = root_0["mills"]; // 1612956904339

long BZunixtime = root_0_date / 1000;
BZunixtime = BZunixtime + (GMTOffset*3600);   // Due  to wintertime in germany as the RTC is set to GMT+1 and not GMT as it normaly schould be
Serial.println("Time saved to file");
Serial.println(BZunixtime);
preferences.putLong("bz_time", BZunixtime); // Put the current Value root_0_date in Bz_time so it can be retrieved next time the function is called

if (bz_time_alt != BZunixtime && BZunixtime != 0 || RTCtime.min == 0 || RTCtime.min == 30) // the display is only updates when a new value exist. Determined by the timestamp. The timestamp is in Unix time. and as a failure catch is !=0
{
canvas.deleteCanvas();  
canvas.createCanvas(330,105);// Size requiered for the display
canvas.setTextSize(200);
char BZ[1]; 
dtostrf(root_0_sgv,3,0,BZ); // convert int to Char BZ
Serial.println("BZ wird aktuallisiert");
canvas.pushCanvas(0,300,UPDATE_MODE_DU4);
canvas.drawString(String(BZ), 0, 0);  


char D[1];
int Delta = round(root_0_delta);
dtostrf(Delta,2,0,D);// convert to the correct file format for display purposes
if (root_0_delta >= 0)
{
canvas.drawString("+",150,0); 
canvas.drawString(D,200,0);
}
else
{
canvas.drawString(D,200,0);
}

// Displays the time of the blood glucose update. in future i want there to be the time of the creation of the value (given from nightscout but having issues getting it to display) 
canvas.setTextSize(5);
sprintf(updatetime,"%02d:%02d",RTCtime.hour,RTCtime.min);
canvas.drawString(updatetime, 0, 70); // position of the time and date
canvas.pushCanvas(0,300,UPDATE_MODE_GC16);


canvas.deleteCanvas();  
canvas.createCanvas(150,150);// Size requiered for the Arrow
canvas.pushCanvas(350,300,UPDATE_MODE_DU4);

if (root_0_delta >= -4 && root_0_delta <= 4 )
{
canvas.fillTriangle(0,0,0,124,150,62,255); //Pfeil Horizontal
}

else if (root_0_delta > 4 && root_0_delta < 10 )
{
canvas.fillTriangle(0,0,150,0,44,106,255); // pfeil 45 hoch
}

else if (root_0_delta >= 10)
{
canvas.fillTriangle(0,150,124,150,62,0,255); //Pfeil nach oben
}

else if (root_0_delta < -4 && root_0_delta > -10 )
{
canvas.fillTriangle(0,106,150,106,44,0,255); // pfeil 45 runter
}
else if (root_0_delta <= -10)
{
canvas.fillTriangle(0,0,62,150,124,0,255); // pfeil nach unten
}

//canvas.fillTriangle(0,0,62,150,124,0,255); // pfeil nach unten
//canvas.fillTriangle(0,0,0,124,150,62,255); //Pfeil Horizontal
//canvas.fillTriangle(0,150,124,150,62,0,255); //Pfeil nach oben
//canvas.fillTriangle(0,0,150,0,44,106,255); // pfeil 45 hoch
//canvas.fillTriangle(0,106,150,106,44,0,255); // pfeil 45 runter
canvas.pushCanvas(350,300,UPDATE_MODE_GC16);

preferences.end();  
}
}
}
}

void  AUS_Battery() // Battery indicator. It can jump a couple % every calculation. Could probably be solved with some sort of filtering or smoothing
{
char v[2];
M5.BatteryADCBegin();
int percent = round(((M5.getBatteryVoltage()-3350)/0.9)/10);
M5.getBatteryVoltage();
canvas.deleteCanvas();  
canvas.createCanvas(235, 30); // Size requiered for the Battery Voltage
canvas.setTextSize(3); 
canvas.drawString("AUS", 0, 0);
if (percent >= 100)
{
canvas.drawString("Akku:CHG", 75, 0);
canvas.pushCanvas(305,10,UPDATE_MODE_GC16);
canvas.deleteCanvas();    
}
else
{
dtostrf(percent,2,1,v); // convert to the correct file format
canvas.drawString("Akku:"+String(v)+"%", 75, 0);  
canvas.pushCanvas(305,10,UPDATE_MODE_GC16); 
canvas.deleteCanvas();
}
}
void MinRefresh () // Updates the Clock when run
{  
canvas.deleteCanvas();  
canvas.createCanvas(540, 45); // Size requiered for the clock
canvas.setTextSize(5); 
canvas.pushCanvas(0,35,UPDATE_MODE_DU4);   //for a clearer screen refresh
flushTime();
canvas.pushCanvas(0,35,UPDATE_MODE_GC16); //actually show the new time

}


void Shutdown() // shutdown script. automatically adjusts the shutdown time to maximize energy saving.
{
Serial.println("Shutting down");
M5.RTC.getTime(&RTCtime);
int timer; 
timer = 59 - RTCtime.sec;
esp_sleep_enable_timer_wakeup(400 * uS_TO_MS_FACTOR);
gpio_hold_en((gpio_num_t) 2); // to prevent the main MOSfet from turning everything off
gpio_deep_sleep_hold_en();
esp_light_sleep_start();
disableEPDPower(); 
gpio_hold_dis((gpio_num_t)2); 
M5.shutdown(timer);// turns of the device, RTC starts the Device again in X-secs. should adjust sleep lenght automatically as long as the task does not take over 1min to complete.
}

void loop() { // the actuall part with determines what runs when. some thing dont need to run every minute. Since it is a e-paper display the information will be retained.  

  M5.RTC.getTime(&RTCtime);
  if (RTCtime.sec <= 3) // to loop every minute
  {
    
 M5.RTC.getTime(&RTCtime); 
if (RTCtime.min == 0)  //if-else loop to determine what to do when, not very elegant but functional
{
  //    enableEPDPower();
       FullCleanDisplay();
       Angeschaltet();
       MinRefresh ();
       UpdateWeather();
       BZdisplay(); 
       AUS_Battery();
       Shutdown();
       
}  
else if (RTCtime.min == 30)
{
   //    enableEPDPower();  
       FullCleanDisplay();
       Angeschaltet();
       MinRefresh ();
       UpdateWeather();
       BZdisplay(); 
       AUS_Battery();
       Shutdown();
}/*  // Disabled for now, looking for a better way of doing things in set intervalls  
else if (RTCtime.min == 15)
{
       Angeschaltet();
   //  FullCleanDisplay();  // for debug purposes
       MinRefresh ();
       Battery();
  //   BZdisplay();
   //  UpdateWeather();    // for debug purposes
       Ausgeschaltet();
       Shutdown();
}       
else if (RTCtime.min == 20)
{
       Angeschaltet();
   //  FullCleanDisplay();  // for debug purposes
       MinRefresh ();
       Battery();
  //   BZdisplay();
   //  UpdateWeather();    // for debug purposes
       Ausgeschaltet();
       Shutdown();
}
else if (RTCtime.min == 25)
{
       Angeschaltet();
   //  FullCleanDisplay();  // for debug purposes
       MinRefresh ();
       Battery();
  //   BZdisplay();
   //  UpdateWeather();    // for debug purposes
       Ausgeschaltet();
       Shutdown();
}
else if (RTCtime.min == 30)
{
       Angeschaltet();
   //  FullCleanDisplay();  // for debug purposes
       MinRefresh ();
       Battery();
  //   BZdisplay();
   //  UpdateWeather();    // for debug purposes
       Ausgeschaltet();
       Shutdown();
}
else if (RTCtime.min == 35)
{
       Angeschaltet();
       FullCleanDisplay();  // for debug purposes
       MinRefresh ();
       Battery();
  //   BZdisplay();
       UpdateWeather();    // for debug purposes
       Ausgeschaltet();
       Shutdown();
}
else if (RTCtime.min == 40)
{
       Angeschaltet();
   //  FullCleanDisplay();  // for debug purposes
       MinRefresh ();
       Battery();
  //   BZdisplay();
   //  UpdateWeather();    // for debug purposes
       Ausgeschaltet();
       Shutdown();
}
else if (RTCtime.min == 45)
{
       Angeschaltet();
   //  FullCleanDisplay();  // for debug purposes
       MinRefresh ();
       Battery();
  //   BZdisplay();
   //  UpdateWeather();    // for debug purposes
       Ausgeschaltet();
       Shutdown();
}
else if (RTCtime.min == 50)
{
       Angeschaltet();
   //  FullCleanDisplay();  // for debug purposes
       MinRefresh ();
       Battery();
  //   BZdisplay();
   //  UpdateWeather();    // for debug purposes
       Ausgeschaltet();
       Shutdown();
}
else if (RTCtime.min == 55)
{
       Angeschaltet();
   //  FullCleanDisplay();  // for debug purposes
       MinRefresh ();
       Battery();
  //   BZdisplay();
   //  UpdateWeather();    // for debug purposes
       Ausgeschaltet();
       Shutdown();
}*/
else 
{    //  enableEPDPower(); 
   //  FullCleanDisplay();  // for debug purposes
       Angeschaltet();
       MinRefresh (); // for now every minute
       BZdisplay();
    // UpdateWeather();    // for debug purposes
       AUS_Battery();
       Shutdown();
}
}
delay(10); // to prevent the watchdog getting trigered
esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_MS_FACTOR); // currently defined as 800ms
gpio_hold_en((gpio_num_t) 2); // to prevent the main MOSfet from turning everything off
gpio_deep_sleep_hold_en();
//Serial.println("went to sleep");
//Serial.flush();
esp_light_sleep_start();
}
