// 2026-03-12 - making it work for dEin-Labor 
// 
// - no reliable WiFi! -> no mqtt, no Raspberry Pi, no near-realtime updates
// 
// 4 branches remaining with numLeds -> scaled (for tiny tree)
// Europe     16  4
// Asia       20  5
// Africa     24  6
// Australia  38  9

#include <FastLED.h>
// https://github.com/FastLED/FastLED/wiki/
// FastLED.setMaxPowerInVoltsAndMilliamps(5, 1800);

#define NUM_LEDS 100 
#define DATA_PIN 27 


// #include <WiFi.h>
// #include <ArduinoOTA.h>

// const char* ssid = "one_solution_revolution";
// const char* password = "Lady_pluS_45";
// const char* ssid = "KarlPhone";
// const char* password = "Aldebaran";

CRGB leds[NUM_LEDS];

void setup() {
  // doesn't work OTA
  Serial.begin(115200); 
  delay(1000); // just to be safe

  // WiFi.begin(ssid, password);
  
  // Initialisiere OTA

  // Setze den Hostnamen für OTA bei mehreren OTA-Geräten - Beispiel: australien
  // ArduinoOTA.setHostname("feather");
  // ArduinoOTA.begin();

  pinMode(DATA_PIN,OUTPUT);
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
  //FastLED.addLeds<SK6812, DATA_PIN, RGB>(leds, NUM_LEDS);
   

  for (int i = 0; i < 5; i++) {
    leds[i] = CRGB(195, 195, 10);  // (stamm)
  }

  // Setze die nächsten 11 LEDs auf gelbgrün
  for (int i = 4; i < NUM_LEDS; i++) {
    leds[i] = CRGB(20, 255, 0);  // (spitze) europas Leutsteifen ist anders codiert als der Rest Europa: (1. blau 2. rot. 3. grün)      Der Rest: (Grün,Rot,Blau)
  }

  
  FastLED.show();
}

void loop() {

  //ArduinoOTA.handle();
  delay(1000);

}