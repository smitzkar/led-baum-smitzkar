// 2026-03-12 - making it work for dEin-Labor 
// 
// - no reliable WiFi! -> no mqtt, no Raspberry Pi, no near-realtime updates
// 
// -[] adjust FastLED.setMaxPowerInVoltsAndMilliamps(5,1000); in setup
// -[] completely replace delay() ? 
// -[] figure out europe strip
//

// 4 branches with varying number of LEDs
// Europe     16  (! uses different led controller !)
// Asia       20
// Africa     24
// Australia  38

// 03-16 platformio (to speed up compiling and get a proper IDE) 
// requirements to make it work:
// - <sketch-name>.ino replaced with main.cpp
// - #include <Arduino.h> added at the top 
// to use the same code via arduino ide:
// - plonk main.cpp back into .ino file
// - deal with any missing libraries via library manager
#include <Arduino.h> 

#include <FastLED.h>
// very simplified "how does this work?"
// (check README for links to documentation)
// - a bunch of set up stuff -> just look at how it's done below or check out example in documentation
// - each individual led gets its own RGB colour, made up of 1 8bit (0-255) brightness per colour  
//  -> for example: 120, 0, 250 (some kind of bright purple)
//  -> in binary: 111100000000000011111010 (just all in one chain)
// - to add more leds, simply add their colour values
//  -> 111100000000000011111010000000000000000000000000 (2nd led is completely off)
//  -> 111100000000000011111010000000000000000000000000111111110000000000000000000 (3rd led is full brightness red)
// - we write this into the FastLED buffer one led (CRGB) at a time,
// - then send the entire package to the first led via FastLED.show()
// - the first led:
//     - reads the first 3 8bit colour values (first 3*8 bits),
//     - removes them from the package, 
//     - and sends the rest to the next one
// - the next led does the same, until there's either no more package remaining or no more led to read it
// so... if we want to only change the n-th led, we still have to resend the data for all the previous ones!      


#define MAX_BRIGHTNESS 200  // 0-255, best to keep this somwhat below max (keeps controllers from overheating)
#define MINS 60000 // little helper

// custom data type for led strips
struct StripConfig {
  const char* name;   // 
  int num_leds;       // number of leds in strip
  int pin;            // data pin
  int milliamps;      // max mA as secondary safety measure
  EOrder color_order; // depends on led-controller
  CRGB* leds;         // points to the actual LED array
  int red_percent;    // for the basic fires display
};

// arrays of CRGB elements (red, green, blue), each 0-255
CRGB leds_europe[16];
CRGB leds_asia[20];
CRGB leds_africa[24];
CRGB leds_australia[38];

// instantiate strips
StripConfig strips[] = {
  {"Europe",    16, 27, 800,  BRG, leds_europe,    15}, // note the different colour order
  {"Asia",      20, 26, 1000, GRB, leds_asia,      30},
  {"Africa",    24, 25, 1200, GRB, leds_africa,    40},
  {"Australia", 38, 33, 1500, GRB, leds_australia, 23}
};
// auto-calculate number of strips 
// very easy to add or remove strips, then forget to manually change this
// so you no longer have to!
// gets size (in bytes) of all strips, then divides by size of single one (they're all of same size)
const int NUM_STRIPS = sizeof(strips) / sizeof(strips[0]); 

// some example colours used in fires display
CRGB colour_healthy = CRGB(195, 195, 10);
CRGB colour_fire = CRGB(255, 20, 0);

// optional: flicker variables
const int INTERVAL_FLICKER_MS = 1*MINS;
unsigned long flicker_end_ms = 0; 
bool use_flicker = false;
bool flicker_active = false;

//MARK: setup
void setup() {
  Serial.begin(115200); 
  delay(1000); // give it some time to start

  Serial.println("=== Multi-Strip LED Controller ===");

  // initialize all strips from config
  for (int i = 0; i < NUM_STRIPS; i++) {
    // set pin mode
    pinMode(strips[i].pin, OUTPUT);
    
    // handle different color orders
    switch(strips[i].color_order) {
      case BRG:
        FastLED.addLeds<WS2812B, strips[i].pin, BRG>(strips[i].leds, strips[i].num_leds)
              .setCorrection(TypicalLEDStrip); // optional, but helps fix some odd colour mismatches
        break;
      case GRB:
        FastLED.addLeds<WS2812B, strips[i].pin, GRB>(strips[i].leds, strips[i].num_leds)
              .setCorrection(TypicalLEDStrip);
        break;
      case RGB:
        FastLED.addLeds<WS2812B, strips[i].pin, RGB>(strips[i].leds, strips[i].num_leds)
              .setCorrection(TypicalLEDStrip);
        break;
        // add default for error handling?
    }
    
    // set max power for strip
    FastLED.setMaxPowerInVoltsAndMilliamps(5, strips[i].milliamps);
    
    Serial.print(strips[i].name);
    Serial.print(" strip initialized (");
    Serial.print(strips[i].num_leds);
    Serial.println(" LEDs)");
  }

  // set brightness scaling for all strips, clear all leds
  FastLED.setBrightness(MAX_BRIGHTNESS);
  FastLED.clear();
  FastLED.show(); 

  // all done! 
  Serial.println("Setup complete, starting loop.");
  printStripStatus();
}



//MARK: loop
void loop() {
  unsigned long now_ms = millis();

  // optional: flicker mode
  if (use_flicker && !flicker_active) {
    Serial.println("Starting flicker mode.");
    flicker_active = true;
    flicker_end_ms = now_ms + INTERVAL_FLICKER_MS;
  }

  if (flicker_active) {
    if (now_ms < flicker_end_ms) {
      // flicker fire all strips
      for (int i = 0; i < NUM_STRIPS; i++) {
        flickeringFire(strips[i].leds, strips[i].num_leds);
        FastLED.show();
        delay(random(40, 600)); // simple attempt to get a more natural flickering
      }
    } else {
      flicker_active = false;
      Serial.println("Flicker mode ended.");
    }
  } else {
    // fallback mode: apply Sara's lights pattern to all strips
    for (int i = 0; i < NUM_STRIPS; i++) {
      sarasLights_karl(strips[i].leds, strips[i].num_leds);
    }
  }

  FastLED.show();  // update all strips at once
  delay(500); 
}

//MARK: LED functions

/* run colour test on all strips (check if red = red, etc.) */
void testColors() {
  Serial.println("Testing RED...");
  for (int i = 0; i < NUM_STRIPS; i++) {
    strips[i].leds[0] = CRGB(255, 0, 0);
  }
  FastLED.show();
  delay(2000);
  
  Serial.println("Testing GREEN...");
  for (int i = 0; i < NUM_STRIPS; i++) {
    strips[i].leds[0] = CRGB(0, 255, 0);
  }
  FastLED.show();
  delay(2000);
  
  Serial.println("Testing BLUE...");
  for (int i = 0; i < NUM_STRIPS; i++) {
    strips[i].leds[0] = CRGB(0, 0, 255);
  }
  FastLED.show();
  delay(2000);
  
  FastLED.clear();
  FastLED.show();
}

/* single colour for whole strip */
void singleColour(CRGB* _leds, int num_leds, int r, int g, int b) {
  for (int i = 0; i < num_leds; i++){
    _leds[i] = CRGB(r, g, b);
  }
}

/* basic gradient effect */
void fancyGradient(CRGB* _leds, int num_leds) {
  for (int i = 0; i < num_leds; i++){
    if (i <= num_leds / 2){
      _leds[i] = CRGB(3*i, 200-4*i, 0);
    }
    else {
      _leds[i] = CRGB(255, 64, 0);
    }
  }
}

/* flickering fire effect */
void flickeringFire(CRGB* _leds, int num_leds) {
  for (int i = 0; i < num_leds; i++) {
    int r = random(120, 255);  
    int g = random(0, 60);
    int b = 0;
    _leds[i] = CRGB(r, g, b);
  }
}

/* 2/3 yellow-green, 1/3 red - Sara's original pattern */
void sarasLights_karl(CRGB* _leds, int num_leds){ 
  int num_green = num_leds * 2 / 3;

  // bottom 2/3: yellow-green (trunk)
  for (int i = 0; i < num_green; i++) {
    _leds[i] = CRGB(195, 195, 10);
  }

  // top 1/3: red (tip)
  for (int i = num_green; i < num_leds; i++) {
    _leds[i] = CRGB(255, 20, 0);
  }
}

/* adjusts branch according to ratio */
void regionFires(CRGB* led_strip, int num_leds, int red_percent = 30){ 
  
  int num_green = num_leds * (100 - red_percent) / 100;  

  // out of bounds check
  if (num_green > num_leds || num_green < 0) {
    Serial.print("ERROR: num_green out of bounds! ");
    Serial.println(num_green);
    return;
  }

  for (int i = 0; i < num_green; i++) {
    led_strip[i] = colour_healthy;  
  }

  for (int i = num_green; i < num_leds; i++) {
    led_strip[i] = colour_fire;  
  }
}


//MARK: helper
/* helper: print strip status */
void printStripStatus() {
  int total_leds = 0;
  int total_milliamps = 0;
  
  Serial.println("\n=== Strip Status ===");
  for (int i = 0; i < NUM_STRIPS; i++) {
    Serial.print(strips[i].name);
    Serial.print(": ");
    Serial.print(strips[i].num_leds);
    Serial.print(" LEDs, ");
    Serial.print(strips[i].milliamps);
    Serial.println("mA limit");
    
    total_leds += strips[i].num_leds;
    total_milliamps += strips[i].milliamps;
  }
  Serial.print("Total: ");
  Serial.print(total_leds);
  Serial.print(" LEDs, ");
  Serial.print(total_milliamps);
  Serial.println("mA budget");
  Serial.println("==================\n");
}

//MARK: 📡 🛰️

/*
* TOP SECRET satellite data function
* somehow works without wlan! some proper NASA-level engineering
* pass in region-led-strip 
* returns percentage of fire
*/
int totallyLegitSatelliteData(StripConfig& strip) {
  
  int current_red_percent = strip.red_percent;
  int change = random(-20, 21);  // magic!
  int new_red_percent = current_red_percent + change;
   
  // add some "gravity" to prevent extremes
  if (new_red_percent > 80) {
    new_red_percent -= random(0, 10);  // pull downward
  } else if (new_red_percent < 20) {
    new_red_percent += random(0, 10);  // pull upward
  }
  
  // ensure that it stays within bounds
  if (new_red_percent < 0) new_red_percent = 0;
  if (new_red_percent > 100) new_red_percent = 100;
  
  Serial.print("🛰️  Satellite data for ");
  Serial.print(strip.name);
  Serial.print(": ");
  Serial.print(current_red_percent);
  Serial.print("% → ");
  Serial.print(new_red_percent);
  Serial.println("%");
  
  return new_red_percent;
}
