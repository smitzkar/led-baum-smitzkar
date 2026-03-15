// 2026-03-12 - making it work for dEin-Labor 
// 
// - no reliable WiFi! -> no mqtt, no Raspberry Pi, no near-realtime updates
// 
// -[] check FastLED.setMaxPowerInVoltsAndMilliamps(5,1000); in setup
// -[]
//
//
// 4 branches remaining with numLeds -> scaled (for tiny tree)
// Europe     16  4  (! uses BRG, not GRB !)
// Asia       20  5
// Africa     24  6
// Australia  38  9

#include <FastLED.h>
// https://github.com/FastLED/FastLED/wiki/
// https://github.com/FastLED/FastLED/wiki/Power-notes
// >Some of these are just good engineering. Some are just plain voodoo. Use whichever ones that work for you.
// > - At the 'far' end of long LED strips, connect the Data line to Ground.
// > - Insert a 200 ohm resistor between the output pins on the microcontroller and the inputs (data, clock) on the LED strip.
// > - Use a level-shifter to raise the voltage of the output pins' 3.3 volts to a full 5 volts before sending it into the LED strip's data (and/or clock) inputs.
// ws2812b datasheet: https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf
// sk6812 datasheet: https://cdn-shop.adafruit.com/product-files/1138/SK6812+LED+datasheet+.pdf 

#define NUM_LEDS 100 
#define DATA_PIN 27 

CRGB leds[NUM_LEDS]; // is set like: leds = CRGB(green, red, blue); // don't ask why

//set colour here for basic test runs (0-255)
int r = 255;
int g = 0;
int b = 0;

//MARK: wifi (optional)
// #include <WiFi.h>
// #include <ArduinoOTA.h>

// const char* ssid = "one_solution_revolution";
// const char* password = "Lady_pluS_45";
// const char* ssid = "KarlPhone";
// const char* password = "Aldebaran";


//MARK: setup
void setup() {
  // doesn't work OTA
  Serial.begin(115200); 
  delay(1000); // give it some time to initialise Serial 

  // WiFi.begin(ssid, password);
  
  // Setze den Hostnamen für OTA bei mehreren OTA-Geräten - Beispiel: australien
  // ArduinoOTA.setHostname("feather");
  // ArduinoOTA.begin();

  pinMode(DATA_PIN,OUTPUT);

  // limit power draw to 1A at 5v (safety measure, adjust accordingly)
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1000); 
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
  //FastLED.addLeds<SK6812, DATA_PIN, RGB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show(); 

}

//MARK: loop
void loop() {

  //ArduinoOTA.handle();

  singleColour();

  delay(10); // just in case I forget to add one in led function 
}


//MARK: led fucntions

void singleColour() {
  for (int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB(g, r, b);
  }

   FastLED.show();
   delay(50);
}

void fancyGradient() {
  for (int i = 0; i < NUM_LEDS; i++){
    if (i <= 10){
      leds[i] = CRGB(3*i, 200-4*i, 0);
    }
    else leds[i] = CRGB(g, r, b);
  }

   FastLED.show();
   delay(50);
}

void flickeringFire() {
  for (int i = 0; i < NUM_LEDS; i++) {
    int r = random(120, 255);  
    int g = random(0, 60); // r-10 ensures that it never ends up too green
    // if (g < 0) g == 0; // shitty, non-robust solution for now
    int b = 0;
    leds[i] = CRGB(g, r, 0);
  }
  FastLED.show();

  //delay(200);
  delay(random(40, 600)); // simple attempt to get a more natural flickering
}

void sarasLights(){
  for (int i = 0; i < 5; i++) {
    leds[i] = CRGB(195, 195, 10);  // (stamm)
  }

  // Setze die nächsten 11 LEDs auf gelbgrün
  for (int i = 4; i < NUM_LEDS; i++) {
    leds[i] = CRGB(20, 255, 0);  // (spitze) europas Leutsteifen ist anders codiert als der Rest Europa: (1. blau 2. rot. 3. grün)      Der Rest: (Grün,Rot,Blau)
  }
  
  FastLED.show();

  delay(1000); // it never changes, so doesn't matter
}
