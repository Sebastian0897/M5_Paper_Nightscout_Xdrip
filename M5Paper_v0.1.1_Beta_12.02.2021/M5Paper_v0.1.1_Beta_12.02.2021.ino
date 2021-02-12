#include <M5EPD.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Arduino_JSON.h>
#include <HTTPClient.h>
#include <Wire.h>

M5EPD_Canvas canvas(&M5.EPD); // setup for the Display

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
//const char* url = "http://192.168.178.75:17580/pebble"; //Blood glukose local Xdrip API, seems to be unreliable, maybe i am doing something wrong
String url = "https://NIGHTSCOUTADRESS/api/v1/entries.json?count=5"; //Blood glucose values from Nightscout. Gets the last 5 Values. currently only using the most recent.
String line; 
char updatetime[10];

void setup() {

    M5.begin();
   // Serial.begin(115200);
   Serial.println("Angeschaltet");
    M5.EPD.SetRotation(90); // for the Screen to be upright
    //M5.EPD.Clear(true);  //if you want a Full refresh after every start
    M5.RTC.begin();         // required for the Clock
}

void flushTime() // Update time from RTC
{   
   
    M5.RTC.getTime(&RTCtime);
    M5.RTC.getDate(&RTCDate);
   
sprintf(timeStrbuff,"%02d/%02d/%d %02d:%02d",
                        RTCDate.day,RTCDate.mon,RTCDate.year,
                        RTCtime.hour,RTCtime.min);
    canvas.drawString(timeStrbuff, 35, 5); // position of the time and date
}


void FullCleanDisplay() // Clean display from possible artifacts
{
 canvas.deleteCanvas();  
canvas.createCanvas(540, 960); 
 canvas.pushCanvas(0,0,UPDATE_MODE_INIT); 
             delay(500);
}



void removeChar(char *str, char garbage) // usefull if you want to remove certain Characters from a String
{
    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
}



void Angeschaltet() // To show that the device is currently working
{
canvas.deleteCanvas();  
canvas.createCanvas(55, 30); // Size requiered for the Display
canvas.setTextSize(3); 
canvas.drawString("AN", 0, 0);  
canvas.pushCanvas(305,10,UPDATE_MODE_GC16);  
}

void Ausgeschaltet() // to show the Device is turned off and saving power
{
canvas.deleteCanvas();  
canvas.createCanvas(55, 30); // Size requiered for the Display
canvas.setTextSize(3); 
canvas.drawString("AUS", 0, 0);  
canvas.pushCanvas(305,10,UPDATE_MODE_GC16); 
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
canvas.deleteCanvas();  
canvas.createCanvas(290, 30); // Size requiered for the Weather
canvas.setTextSize(3); 
canvas.drawString("NICHT Verbunden", 0, 0);  
canvas.pushCanvas(10,10,UPDATE_MODE_GC16);               
}
if(WiFi.status()== WL_CONNECTED){// show that it connected to a WIFI network
canvas.deleteCanvas();  
canvas.createCanvas(290, 30); // Size requiered for the Weather
canvas.setTextSize(3); 
canvas.drawString("WLAN Verbunden", 0, 0);  
canvas.pushCanvas(10,10,UPDATE_MODE_GC16); 
}
}

void UpdateWeather()  // Get weather info
{
ConnectWifi();
if(WiFi.status()== WL_CONNECTED){
      String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&lang="  + lang   + "&units=" + units + "&APPID=" + openWeatherMapApiKey;      
      JsonBuffer1 = httpGETRequest(serverPath.c_str());
      Serial.println(JsonBuffer1); // you cann see the string in an serial monitor to implement more variables. there a quite a few that can be used
      JSONVar myObject = JSON.parse(JsonBuffer1);

canvas.deleteCanvas();  
canvas.createCanvas(400, 40); // Size requiered for the Weather
canvas.setTextSize(4); 
char temp[5];
dtostrf(myObject["main"]["temp"],5,2,temp); // convert to the correct file format
canvas.drawString("Temperatur:"+String(temp), 0, 0);  
canvas.pushCanvas(10,100,UPDATE_MODE_GC16);


/*word bild = (word)myObject["weather"]["icon"];                // Work in Progress to get small weather icons. need a way of sorting which image to load. havent figured that out yet
canvas.deleteCanvas();  
canvas.createCanvas(100, 100);
const char* Url = "http://openweathermap.org/img/wn/13d@2x.png";
canvas.drawPngUrl(Url);
canvas.pushCanvas(410,100,UPDATE_MODE_GC16);
*/
}
}

String httpGETRequest(const char* serverName) { // this is used to write the information from the webservices into a string. needed for weather and Nightscout
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
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
  //  Serial.print("Error code: ");
  //  Serial.println(httpResponseCode);
/*canvas.deleteCanvas();                     //Debug if required
canvas.createCanvas(540, 40); // Size requiered
canvas.setTextSize(5); 
canvas.drawString("Fehler", 0, 0);  
canvas.pushCanvas(0,100,UPDATE_MODE_GC16);  */
  }
  // Free resources
  http.end();
  return payload;
}

 
void BZdisplay() // Gets the current blood glucose from nightscout and displays it.
{
ConnectWifi();
if(WiFi.status()!= WL_CONNECTED){
  Ausgeschaltet();
  Shutdown();
}
       
 // create HTTP Client
      JsonBuffer2 = httpGETRequest(url.c_str());
      Serial.println(JsonBuffer2);
      
  /*    int n = JsonBuffer2.length();  // used to remove unwanted characters from the string
     char char_array[n + 1];
     /strcpy(char_array, JsonBuffer2.c_str());    
       removeChar(char_array, ':');         
       Serial.println(char_array);
      JSONVar myObject1 = JSON.parse(char_array);*/
      
   JSONVar myObject1 = JSON.parse(JsonBuffer2);
     if (JSON.typeof(myObject1) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
        }
canvas.deleteCanvas();  
canvas.createCanvas(300,60);// Size requiered for the display
canvas.setTextSize(200);
//Serial.println(myObject1[0]["sgv"]);
char BZ[5]; // bigger array for later graphing maybe. 
dtostrf(myObject1[0]["sgv"],3,0,BZ); // convert to the correct file format
canvas.drawString(String(BZ), 0, 0);  

char D[1];
dtostrf(myObject1[0]["delta"],2,0,D);// convert to the correct file format for display purposes
int Richtung = atoi(D);
Serial.println(Richtung);
if (Richtung >= 0)
{
canvas.drawString("+",150,0); 
canvas.drawString(D,200,0);
}
else
{
  canvas.drawString(D,200,0);
}
canvas.pushCanvas(0,300,UPDATE_MODE_GC16);

// Displays the time of the blood glucose update. in future i want there to be the time of the creation of the value (given from nightscout but having issues getting it to display)
canvas.deleteCanvas();  
canvas.createCanvas(150,45);// Size requiered for the display
canvas.setTextSize(5);
sprintf(updatetime,"%02d:%02d",RTCtime.hour,RTCtime.min);
canvas.drawString(updatetime, 0, 0); // position of the time and date
canvas.pushCanvas(0,370,UPDATE_MODE_GC16);

canvas.deleteCanvas();  
canvas.createCanvas(150,150);// Size requiered for the display
canvas.setTextSize(200);

if (Richtung >= -4 && Richtung <= 4 )
{
canvas.fillTriangle(0,0,0,124,150,62,255); //Pfeil Horizontal
}

else if (Richtung >= 5 && Richtung < 10 )
{
canvas.fillTriangle(0,0,150,0,44,106,255); // pfeil 45 hoch
}

else if (Richtung >= 10)
{
//canvas.fillTriangle(0,150,124,150,62,0,255); //Pfeil nach oben
}

else if (Richtung <= -5 && Richtung > -10 )
{
//canvas.fillTriangle(10,0,10,150,160,75,255);
canvas.fillTriangle(0,106,150,106,44,0,255); // pfeil 45 runter
}
else if (Richtung <= -10)
{
canvas.fillTriangle(0,0,62,150,124,0,255); // pfeil nach unten
}

Serial.println(myObject1[0]["date"]);
//canvas.fillTriangle(0,0,62,150,124,0,255); // pfeil nach unten
//canvas.fillTriangle(0,0,0,124,150,62,255); //Pfeil Horizontal
//canvas.fillTriangle(0,150,124,150,62,0,255); //Pfeil nach oben
//canvas.fillTriangle(0,0,150,0,44,106,255); // pfeil 45 hoch
//canvas.fillTriangle(0,106,150,106,44,0,255); // pfeil 45 runter
/*char Time[1]; // To show the Time of the last value
dtostrf(myObject1[0]["date"],3,0,Time); // convert to the correct file format
canvas.setTextSize(3);
canvas.drawString(String(Time), 0, 50); 
*/
canvas.pushCanvas(350,300,UPDATE_MODE_GC16);
}




void Battery() // Battery indicator. It can jump a couple % every calculation. Could probably be solved with some sort of filtering or smoothing
{
char v[2];
M5.BatteryADCBegin();
float percent = ((M5.getBatteryVoltage()-3000)/1.25)*100;
M5.getBatteryVoltage();
canvas.deleteCanvas();  
canvas.createCanvas(180, 30); // Size requiered for the Battery Voltage
canvas.setTextSize(3); 
dtostrf(percent,2,1,v); // convert to the correct file format
canvas.drawString("Akku:"+String(v)+"%", 0, 0);  
canvas.pushCanvas(380,10,UPDATE_MODE_GC16); 
canvas.deleteCanvas();
}

void MinRefresh () // Updates the Clock when run
{
canvas.deleteCanvas();  
canvas.createCanvas(540, 45); // Size requiered for the clock
canvas.setTextSize(5); 
canvas.pushCanvas(0,35,UPDATE_MODE_GC16);   //for a clearer screen refresh
delay(400);
flushTime();
canvas.pushCanvas(0,35,UPDATE_MODE_GC16); //actually show the new time

}


void Shutdown() // shutdown script. automatically adjusts the shutdown time to maximize energy saving.
{
delay(500); // to ensure everything has finished
M5.RTC.getTime(&RTCtime);
int timer = 56- RTCtime.sec;
M5.shutdown(timer);// turns of the device, RTC starts the Device again in X-secs. should adjust sleep lenght automatically as long as the task does not take over 1min to complete. has a safety margin of 4 secs. 2 of those seconds are actually required to boot 
}

void loop() { // the actuall part with determines what runs when. some thing dont need to run every minute. Since it is a e-paper display the information will be retained.
 
  M5.RTC.getTime(&RTCtime);
  if (RTCtime.sec == 0) // to update time every minute
  {
 M5.RTC.getTime(&RTCtime); 
if (RTCtime.min == 5)  //if-else loop to determine what to do when, not very elegant but functional
{
       FullCleanDisplay();
       Angeschaltet();
       MinRefresh ();
       Battery();
       BZdisplay();
       UpdateWeather();    
       Ausgeschaltet();
       Shutdown();
}/*      // Disabled for now, looking for a better way of doing things in set intervalls
else if (RTCtime.min == 10)
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
{  
   //  FullCleanDisplay();  // for debug purposes
       Angeschaltet();
       MinRefresh (); // for now every minute
       Battery();
       BZdisplay();
   //  UpdateWeather();    // for debug purposes
       Ausgeschaltet();
       Shutdown();
}


 delay(300);  // so it does not try the first if-loop so fast and often. energy saving, I think?
}
}
