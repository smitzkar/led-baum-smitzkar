// 2026-03-12 - making it work for dEin-Labor 
// 
// - no reliable WiFi! -> no mqtt, no Raspberry Pi, no near-realtime updates
// 
// -[] adjust FastLED.setMaxPowerInVoltsAndMilliamps(5,1000); in setup
// -[] completely replace delay() ? 
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

#define NUM_LEDS 20
#define DATA_PIN 27 
#define MINS 60000 // minutes in milliseconds -> just to make things easier
#define POTI_PIN 36
#define COLOUR_ORDER GRB // depends on controller
#define LED_TYPE WS2812B
#define MAX_BRIGHTNESS 200 // 0-255 -> lower this to avoid overheating (which messes with colours -> defaults to reds)
#define MAX_MILLIAMPS 1000 

CRGB leds[NUM_LEDS];

//set colour here for basic test runs (0-255)
int r = 255;
int g = 15;
int b = 64;

int brightness = 127; // 0-255
int poti_value = 0;

//MARK: wifi (optional)
// #include <WiFi.h>
// #include <ArduinoOTA.h>

// const char* ssid = "one_solution_revolution";
// const char* password = "Lady_pluS_45";
// const char* ssid = "KarlPhone";
// const char* password = "Aldebaran";

// forward declaration
void flickeringFire(int min, int max);  
const int INTERVAL_FLICKER_MS = 1*MINS;
int flicker_end_ms = 0; 
bool use_flicker = true;
bool flicker_active = false;

//MARK: setup
void setup() {
  // doesn't work OTA
  Serial.begin(115200); 
  delay(1000); // give it some time to initialise Serial 
  Serial.println("Serial set up.");

  // WiFi.begin(ssid, password);
  
  // Setze den Hostnamen für OTA bei mehreren OTA-Geräten - Beispiel: australien
  // ArduinoOTA.setHostname("feather");
  // ArduinoOTA.begin();

  pinMode(DATA_PIN,OUTPUT);
  //pinMode(POTI_PIN,INPUT);


  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOUR_ORDER>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_MILLIAMPS); // limit power draw 
  FastLED.setBrightness(MAX_BRIGHTNESS);
  FastLED.clear();
  FastLED.show(); 

  Serial.println("Set up complete, starting loop.");

}

//MARK: loop
void loop() {

  //ArduinoOTA.handle();

  // read poti to set brightness
  // poti_value = analogRead(POTI_PIN); // default = 0-4095 (12bit) for esp32's DAC
  // Serial.println(poti_value);
  // brightness = poti_value / 16; // bring it to 0-255 (8bit). integer division is default on cpp
  // FastLED.setBrightness(brightness); // takes uint_8 value and scales brightness accordingly
  // // this doesn't work as expected... 
  // Serial.println(brightness);
  
  //testColors();


  // singleColour(g,r,b);
  // delay(5000);
  // fancyGradient(g,r,b);
  // delay(5000);

  // non-blocking flicker
  // actually counter-productive here...
  use_flicker = false;
  unsigned long now_ms = millis();
  if (use_flicker && !flicker_active) {
    Serial.println("Starting flicker.");
    flicker_active = true;
    flicker_end_ms = now_ms + INTERVAL_FLICKER_MS;
  }
  if (flicker_active) {
    if (now_ms < flicker_end_ms) {
      flickeringFire(30, 100);
    } else {
      flicker_active = false;
    }
  }

  sarasLights_karl(leds, NUM_LEDS);
  delay(500);

  delay(10); // just in case I forget to add one in led function 
}


//MARK: led fucntions

/*cycles through colours of first led -> red, green, blue*/
void testColors() {
  leds[0] = CRGB(255, 0, 0);
  FastLED.show();
  delay(2000);
  
  leds[0] = CRGB(0, 255, 0);
  FastLED.show();
  delay(2000);
  
  leds[0] = CRGB(0, 0, 255);
  FastLED.show();
  delay(2000);
}

void singleColour(int g, int r, int b) {
  for (int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB(g, r, b);
  }

   FastLED.show();
   delay(50);
}


void fancyGradient(int g, int r, int b) {
  for (int i = 0; i < NUM_LEDS; i++){
    if (i <= 10){
      leds[i] = CRGB(3*i, 200-4*i, 0);
    }
    else leds[i] = CRGB(g, r, b);
  }

   FastLED.show();
   delay(50);
}

/*uses pre-set rgb values, flickers with randomised delay between min and max ms*/
void flickeringFire(int min = 40, int max = 600) {
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

/*uses pre-set rgb values and number of leds*/
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

/*2/3 yellow-green, 1/3 red*/
void sarasLights_karl(CRGB* _leds, int num_leds){ 

  int num_green = num_leds * 2 / 3;

  for (int i = 0; i < num_green; i++) {
    //_leds[i] = CRGB::GreenYellow;  // (stamm)
    _leds[i] = CRGB(195, 195, 10);  // this should be red-green ? 
  }

  for (int i = num_green; i < num_leds; i++) {
    //_leds[i] = CRGB::Red; 
    _leds[i] = CRGB(255, 20, 0);  
  }
  
  FastLED.show();

  delay(1000); // it never changes, so doesn't matter
}
