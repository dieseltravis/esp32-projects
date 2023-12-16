/*
 Set Latest CheerLights Color on a RGB LED via ESP32 Wi-Fi

 This sketch periodically checks the CheerLights color that is stored in
 ThingSpeak channel 1417 and sets the color of a an RGB LED.

 Requirements:

   * ESP32 Wi-Fi Device
   * Arduino 1.8.8+ IDE

 Setup Wi-Fi:
  * Enter SECRET_SSID in "secrets.h"
  * Enter SECRET_PASS in "secrets.h"

 CheerLights Project: http://cheerlights.com
 Original Created: Dec 13, 2023 by Hans Scharler (http://nothans.com)
 ESP32 remix by Travis Hardiman
*/
#include <Arduino.h>
#include <WiFi.h>
#include <ThingSpeak.h>
#include <Adafruit_NeoPixel.h>
#include "secrets.h"

unsigned long cheerLightsChannelNumber = 1417;

// set this if ESP32 has an on-board RGB LED
#define ESP_RGB_LED 47   // S2 15

#define ESP_LED_BLUE 21
#define ESP_LED_RED 15 // S2 34
#define ESP_LED_GREEN 17

// Define the supported CheerLights colors and their RGB values
String colorName[] = {"red", "pink", "green", "blue", "cyan", "white", "warmwhite", "oldlace", "purple", "magenta", "yellow", "orange"};

int colorRGB[][3] = { 255,   0,   0,  // "red"
                      255, 192, 203,  // "pink"
                        0, 255,   0,  // "green"
                        0,   0, 255,  // "blue"
                        0, 255, 255,  // "cyan"
                      255, 255, 255,  // "white"
                      255, 223, 223,  // "warmwhite" 
                      255, 223, 223,  // "oldlace"
                      128,   0, 128,  // "purple"
                      255,   0, 255,  // "magenta"
                      255, 255,   0,  // "yellow"
                      255, 165,   0}; // "orange"

const char* ssid = SECRET_SSID;   // your network SSID (name)
const char* password = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key index number (needed only for WEP)
WiFiClient  client;
Adafruit_NeoPixel pixel = Adafruit_NeoPixel(1, ESP_RGB_LED, NEO_RGB + NEO_KHZ800); // Set up the NeoPixel library

String color = "white";
String lastColor = "black";

int r, g, b, lr = 0, lg = 0, lb = 0;

void setup() {
  Serial.begin(115200);
  delay(100);
  pixel.begin();
  
  Serial.print("ESP_RGB_LED: ");
  Serial.println(ESP_RGB_LED);

  Serial.print("LED_BUILTIN: ");
  Serial.println(LED_BUILTIN);

  Serial.print("RGB_BUILTIN: ");
  Serial.println(RGB_BUILTIN);
    
  pinMode(ESP_LED_RED, OUTPUT);
  pinMode(ESP_LED_GREEN, OUTPUT);
  pinMode(ESP_LED_BLUE, OUTPUT);
  
  pixel.setBrightness(50);

  // Red LED 
  digitalWrite(ESP_LED_RED, HIGH); 
  digitalWrite(ESP_LED_GREEN, LOW);
  digitalWrite(ESP_LED_BLUE, LOW);
  pixel.setPixelColor(0, pixel.Color(255, 0, 0));
  pixel.show();
  delay(1000);
  
  // Green LED 
  digitalWrite(ESP_LED_RED, LOW); 
  digitalWrite(ESP_LED_GREEN, HIGH);
  digitalWrite(ESP_LED_BLUE, LOW);
  pixel.setPixelColor(0, pixel.Color(0, 255, 0));
  pixel.show();
  delay(1000);
  
  //neopixelWrite(RGB_BUILTIN,0,0,255); // Blue
    // Blue LED 
  digitalWrite(ESP_LED_RED, LOW); 
  digitalWrite(ESP_LED_GREEN, LOW);
  digitalWrite(ESP_LED_BLUE, HIGH);
  pixel.setPixelColor(0, pixel.Color(0, 0, 255));
  pixel.show();
  delay(1000);
  
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(SECRET_SSID);
  Serial.print("ESP MAC address: ");
  Serial.println(WiFi.macAddress());

  pixel.setPixelColor(0, pixel.Color(255, 223, 223));
  pixel.show();

  // I think STA is default
  // WiFi.mode(WIFI_STA);

  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
  WiFi.begin(ssid, password);

  pixel.setPixelColor(0, pixel.Color(255, 255,   0));
  pixel.setBrightness(100);
  pixel.show();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    // non rgb led: 
    //digitalWrite(ESP_LED, HIGH);
    // rgb led
    pixel.setPixelColor(0, pixel.Color(255, 255,   0));
    pixel.show();
    delay(50);
    
    //digitalWrite(ESP_LED, LOW);
    pixel.clear();
    pixel.setPixelColor(0, pixel.Color(0, 0, 0));
    pixel.show();
    delay(100);
    
    Serial.print(".");
    //digitalWrite(ESP_LED, HIGH);
    pixel.setPixelColor(0, pixel.Color(255, 255,   0));
    pixel.show();
    delay(50);
    Serial.print(WiFi.status());

    //digitalWrite(ESP_LED, LOW);
    pixel.clear();
    pixel.setPixelColor(0, pixel.Color(0, 0, 0));
    pixel.show();
    delay(500);
  }
  Serial.println("\nConnected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  ThingSpeak.begin(client);
  //digitalWrite(ESP_LED, LOW);
  pixel.clear();
  digitalWrite(ESP_LED_RED, LOW); 
  digitalWrite(ESP_LED_GREEN, LOW);
  digitalWrite(ESP_LED_BLUE, LOW);

  Serial.println("setting red...");
  setColor("red");
  delay(500);
  Serial.println("setting green...");
  setColor("green");
  delay(500);
  Serial.println("setting blue...");
  setColor("blue");
  delay(500);
  Serial.println("setting purp...");
  setColor("purple");
  delay(500);
  Serial.println("setting white...");
  setColor("white");
  delay(500);
}

void loop() {
  int statusCode = 0;
  
  // Read CheerLights color from ThingSpeak channel
  color = ThingSpeak.readStringField(cheerLightsChannelNumber, 1);

  // Check the status of the read operation to see if it was successful
  statusCode = ThingSpeak.getLastReadStatus();

  if (statusCode == 200) {
    Serial.println("Latest CheerLights Color: " + color);
    if (lastColor != color) {
      Serial.println("changing from " + lastColor + " to " + color);
      setColor(color);
      lastColor = color;
    }
  }
  else {
    Serial.println("Problem reading channel. HTTP error code " + String(statusCode));
    //
  }

  // Wait 20 seconds before checking again
  delay(20000);
  // todo: breathe?
}

void setColor(String color) {
  for (int colorIndex = 0; colorIndex < 12; colorIndex++) {
    if (color == colorName[colorIndex]) {
      r = colorRGB[colorIndex][0];
      g = colorRGB[colorIndex][1];
      b = colorRGB[colorIndex][2];

      pixel.setPixelColor(0, pixel.Color(r, g, b));
      pixel.show();
      
      // todo: fade out ?
      // todo: fade in ?

      // fade to color:
      // delta values
      int dr = (r == lr) ? 0 : (r - lr > 0) ? 1 : -1;
      int dg = (g == lg) ? 0 : (g - lg > 0) ? 1 : -1;
      int db = (b == lb) ? 0 : (b - lb > 0) ? 1 : -1;

      // current values
      int cr = lr;
      int cg = lg;
      int cb = lb;
      
      // loop until all equal
      while (cr != r || cg != g || cb != b) {
        analogWrite(ESP_LED_RED, cr); 
        analogWrite(ESP_LED_GREEN, cg);
        analogWrite(ESP_LED_BLUE, cb);
        delay(20);
        if (cr != r) cr += dr;
        if (cg != g) cg += dg;
        if (cb != b) cb += db;
      }
    
      lr = r;
      lg = g;
      lb = b;

      pixel.clear();
      pixel.setPixelColor(0, pixel.Color(0, 0, 0));
      pixel.show();
      break;
    }
  }
}
