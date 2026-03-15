// 2026-03-12 - making it work for dEin-Labor 
// 
// - no reliable WiFi! -> no mqtt, no Raspberry Pi, no near-realtime updates 
// 
// -[] adjust FastLED.setMaxPowerInVoltsAndMilliamps(5,1000); in setup
// -[] completely replace delay() ? 
// -[] figure out europe strip
//   -[] 
//
//
// 4 branches remaining with numLeds and scaled (for tiny tree)
// Europe     16  4  (! uses different led controller! - also has 50 leds? !)
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

#define NUM_LEDS 8
#define DATA_PIN 27 
#define MINS 60000 // minutes in milliseconds -> just to make things easier
#define COLOUR_ORDER GRB // depends on controller
#define LED_TYPE WS2812B
#define MAX_BRIGHTNESS 200 // 0-255 -> lower this to avoid overheating (which messes with colours -> defaults to reds)
#define MAX_MILLIAMPS 1000 

CRGB leds[NUM_LEDS];

// old: set colour here for basic test runs (0-255)
int r = 255;
int g = 15;
int b = 64;

CRGB colour_healthy = CRGB(195, 195, 10);
CRGB colour_fire = CRGB(255, 20, 0);

// forward declaration
void flickeringFire();  
const int INTERVAL_FLICKER_MS = 1*MINS;
unsigned long flicker_end_ms = 0; 
bool use_flicker = false; // enable/disable here
bool flicker_active = false;

void twoColourBranch(CRGB* led_strip, int num_leds, int red_percent);

//MARK: setup
void setup() {

  Serial.begin(115200); 
  delay(1000); // give it some time to initialise Serial 
  Serial.println("Serial set up.");

  pinMode(DATA_PIN,OUTPUT);

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOUR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_MILLIAMPS); // limit power draw 
  FastLED.setBrightness(MAX_BRIGHTNESS);
  FastLED.clear();
  FastLED.show(); 

  Serial.println("Set up complete, starting loop.");

}

//MARK: loop
void loop() {

  //testColors();

  // singleColour(g,r,b);
  // FastLED.show();
  // delay(5000);
  // fancyGradient(g,r,b);
  // FastLED.show();
  // delay(5000);

  // non-blocking flicker
  // actually counter-productive here, so added in a blocking delay again...
  unsigned long now_ms = millis();
  if (use_flicker && !flicker_active) {
    Serial.println("Starting flicker.");
    flicker_active = true;
    flicker_end_ms = now_ms + INTERVAL_FLICKER_MS;
  }
  if (flicker_active) {
    if (now_ms < flicker_end_ms) {
      flickeringFire();
      FastLED.show();
      delay(random(40, 600)); // simple attempt to get a more natural flickering
    } else {
      flicker_active = false;
    }
  }

  if (!use_flicker){
    twoColourBranch(leds, NUM_LEDS, 30);
    FastLED.show();
    delay(50);
  }

  delay(50); // just in case I forget to add one in led function 
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
    leds[i] = CRGB(r,g,b);
  }
}


void fancyGradient(int g, int r, int b) {
  for (int i = 0; i < NUM_LEDS; i++){
    if (i <= 10){
      leds[i] = CRGB(3*i, 200-4*i, 0);
    }
    else leds[i] = CRGB(r, g, b);
  }
   FastLED.show();
   delay(50);
}

/*uses pre-set rgb values, randomises them (use with randomised delay)*/
void flickeringFire() {
  for (int i = 0; i < NUM_LEDS; i++) {
    int r = random(120, 255);  
    int g = random(0, 60); // r-10 ensures that it never ends up too green
    // if (g < 0) g == 0; // shitty, non-robust solution for now
    int b = 0;
    leds[i] = CRGB(r, g, 0);
    
    // delay(random(40, 600)); // simple attempt to get a more natural flickering
  }
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
}

/*adjusts branch according to ratio*/
void twoColourBranch(CRGB* led_strip, int num_leds, int red_percent = 30){ 
  
  int num_green = num_leds * ( 100 - red_percent ) / 100;

  for (int i = 0; i < num_green; i++) {
    led_strip[i] = colour_healthy;  
  }

  for (int i = num_green; i < num_leds; i++) {
    led_strip[i] = colour_fire;  
  }
}

/*
* somehow works without wlan! some proper NASA-level engineering
* returns percentage of fire
*/
int totallyLegitSatelliteData(int region_id){ 
  // take current red_percent, adjust it randomly
  int new_red_percent = current_red_percent - random(-20, 20); 
  // maybe adjust this a bit, so that it "tries" to stay within certain ranges? 
  return 5;
}
