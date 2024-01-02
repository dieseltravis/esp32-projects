/*******************************************************************
    A simple radio player for the ESP32 Cheap Yellow Display.

    Note: This sketch can play most radio streams, but it might be hard to 
    find the exact url to use. 
    You can some url's on the following sites:
    
    - https://streamurl.link/ 
    - http://listenlive.nl/
    - https://en.wikipedia.org/wiki/List_of_Internet_radio_stations/
    - https://github.com/mikepierce/internet-radio-streams/

    Also note that many streams are geographically restricted as well.

    https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display

    If you find what Brian Lough do useful and would like to support him,
    please consider becoming a sponsor on Github
    https://github.com/sponsors/witnessmenow/

    Written by Jonny Bergdahl
    YouTube: https://www.youtube.com/jonnybergdahl
    Twitter: https://twitter.com/jonnybergdahl
 *******************************************************************/

// Make sure to copy the UserSetup.h file into the library as
// per the Github Instructions. The pins are defined in there.

// ----------------------------
// Standard Libraries
// ----------------------------

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <SPI.h>

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <TFT_eSPI.h>
// A library for interfacing with LCD displays
//
// Can be installed from the library manager (Search for "TFT_eSPI")
//https://github.com/Bodmer/TFT_eSPI

#include <VS1053.h>               /* https://github.com/baldram/ESP_VS1053_Library */
// https://github.com/CelliesProjects/ESP32_VS1053_Stream
#include <ESP32_VS1053_Stream.h>

// ----------------------------

// VS1053 pins:
#define VS1053_XRST    -1
#define VS1053_RST     -1

// SD DATA0  IO19   DO / MISO
#define VS1053_MISO    19
//#define MISO         19

// SD CMD    IO23   DI / MOSI
#define VS1053_MOSI    23
//#define MOSI         23

// SD CLK    IO18   SCLK
#define VS1053_SCLK    18
#define VS1053_CLK     18
//#define CLK          18

// CN1       IO22
#define VS1053_DREQ    22

// SD CD     IO5  CS
#define VS1053_XCS     5
#define VS1053_CS      5

// CN1       IO27
#define VS1053_XDCS    27
#define VS1053_DCS     27

// Change ssid and password for your local WiFi in secrets.h
#include "secrets.h"
//#define SECRET_SSID "secretwifiname"
//#define SECRET_PASS "secretpassword"
const char* ssid = SECRET_SSID;   // your network SSID (name)
const char* password = SECRET_PASS;   // your network password

const char* stations[] = {
  //"https://streaming.wayofm.org/wayo-192",
  //"http://streaming.wayofm.org:8000/wayo-192.m3u",
  //"http://streaming.wayofm.org:8000/wayo-192",
  "http://streaming.wayofm.org:8000/wayo-lite.m3u",
  //"http://streaming.wayofm.org:8000/wayo-lite",
  "http://streaming.wayofm.org:8000/wayo-mobile.m3u",
  //"http://streaming.wayofm.org:8000/wayo-mobile"
};
#define STATION_LEN (sizeof(stations) / sizeof(stations[0]))

TFT_eSPI tft = TFT_eSPI();
ESP32_VS1053_Stream stream;

void setup() {
  Serial.begin(115200);
  delay(500);
  
  // Connect to WiFi
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  Serial.print("ESP MAC address: ");
  Serial.println(WiFi.macAddress());
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    Serial.print(WiFi.status());
    delay(500);
  }
  Serial.println("\nConnected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //Serial.println("Init wire...");
  //Wire.begin(VS1053_DREQ, VS1053_DCS);
  
  Serial.println("Init SPI...");
  Serial.println("SPI.setHwCs...");
  SPI.setHwCs(true);
  //SPI.begin(SPI_CLK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);  /* start SPI before starting decoder */
  Serial.println("SPI.begin...");
  SPI.begin(VS1053_SCLK, VS1053_MISO, VS1053_MOSI);
  delay(5000);
  
  //pinMode(VS1053_RST, OUTPUT);
  //digitalWrite(VS1053_RST, HIGH);

  Serial.println("Init VS1053...");
  while (!stream.startDecoder(VS1053_CS, VS1053_DCS, VS1053_DREQ)) {
    Serial.println("Decoder not running");
    delay(1000);
  }
  /*
  while (!stream.isChipConnected()) {
    Serial.println("Decoder not connected");
    delay(1000);
    stream.startDecoder(VS1053_CS, VS1053_DCS, VS1053_DREQ);
  }
  */
  // Start the TFT display and set it to black
  tft.init();
  tft.setRotation(1); //This is the display in landscape
  tft.setTextWrap(true, true);

  // Clear the screen before writing to it and set default text colors
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  
  tft.drawString("Title...", 0, 10, 1);

  Serial.println("decoder running - starting stream");
  bool streamSuccess = false;
  int streamIndex = 0;
  
  while (!streamSuccess && streamIndex < STATION_LEN) {
    Serial.print("Trying stream index: ");
    Serial.print(streamIndex);
    Serial.print(" of ");
    Serial.print(STATION_LEN);
    Serial.print(": ");
    Serial.println(stations[streamIndex]);
    streamSuccess = stream.connecttohost(stations[streamIndex]);
    streamIndex++;
  }

  if (streamSuccess) {
    Serial.print("codec: ");
    Serial.println(stream.currentCodec());
  
    Serial.print("bitrate: ");
    Serial.print(stream.bitrate());
    Serial.println("kbps");
    
    Serial.print("volume: ");
    Serial.println(stream.getVolume());
  } else {
    Serial.println("Unsuccessful.");
  }

}

void loop() {
  stream.loop();
  /*
  Serial.print("isChipConnected: ");
  Serial.print(stream.isChipConnected());
  Serial.print(" isRunning: ");
  Serial.print(stream.isRunning());
  Serial.printf(" Buffer status: %s\n", stream.bufferStatus());
  delay(500);
  */
  delay(10);
}

void printTitle(const char* info) {
  tft.fillRect(0, 20, 320, 200, TFT_BLACK);
  tft.setCursor(0, 20, 4);
  tft.setTextColor(TFT_SKYBLUE);
  tft.println(info);
}

void printInfo(const char* info) {
  tft.fillRect(0, 230, 320, 10, TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawString(info, 0, 230, 1);
}

void audio_info(const char *info) {
  Serial.print("info        "); 
  Serial.println(info);
  printInfo(info);
}

void audio_id3data(const char *info) {  //id3 metadata
  Serial.print("id3data     ");
  Serial.println(info);
}

void audio_eof_mp3(const char *info) {  //end of file
  Serial.print("eof_mp3     ");
  Serial.println(info);
}

void audio_eof_stream(const char* info) {
  Serial.printf("eof: %s\n", info);
}

void audio_showstation(const char *info) {
  Serial.print("station     ");
  Serial.println(info);
  printInfo(info);
}

void audio_showstreamtitle(const char *info) {
  Serial.print("streamtitle ");
  Serial.println(info);
  printTitle(info);
}

void audio_bitrate(const char *info) {
  Serial.print("bitrate     ");
  Serial.println(info);
}

void audio_commercial(const char *info) {  //duration in sec
  Serial.print("commercial  ");
  Serial.println(info);
}

void audio_icyurl(const char *info) {  //homepage
  Serial.print("icyurl      ");
  Serial.println(info);
}

void audio_lasthost(const char *info) {  //stream URL played
  Serial.print("lasthost    ");
  Serial.println(info);
}

void audio_eof_speech(const char *info) {
  Serial.print("eof_speech  ");
  Serial.println(info);
}
