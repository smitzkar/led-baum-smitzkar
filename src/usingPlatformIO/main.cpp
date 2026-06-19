// 2026-03-12 - making it work for dEin-Labor 
// 
// - no reliable WiFi! -> no mqtt, no Raspberry Pi, no near-realtime updates
// 
// -[] adjust FastLED.setMaxPowerInVoltsAndMilliamps(5,1000); in setup
// -[x] figure out europe strip
//

// 4 branches with varying number of LEDs
// Europe     17
// Asia       20
// Africa     24
// Australia  38

// 2026-03-16 platformio (to speed up compiling and get a proper IDE) 
// requirements to make it work:
// - <sketch-name>.ino replaced with main.cpp
// - #include <Arduino.h> added at the top 
// to use the same code via arduino ide:
// - plonk main.cpp back into <sketchname>.ino file
// - if relevant: add any .h and .cpp files to sketch folder (where <sketchname.ino> lives)
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

// pin definitions (only way to NOT have to hardcode them inside setup)
#define EURO_PIN   27
#define ASIA_PIN   26
#define AFRI_PIN   25
#define AUST_PIN   33
#define TEST_PIN   32
// colour order definitions (same deal)
#define EURO_ORDER GRB
#define ASIA_ORDER GRB
#define AFRI_ORDER GRB
#define AUST_ORDER GRB
#define TEST_ORDER GRB


// uncomment ONE of these to select configuration
// (using conditional compilation / preprocessor conditionals -> the other simply aren't compiled at all):
#define TINY_TREE
// #define FULL_TREE

// number of leds
#ifdef TINY_TREE
  #define EURO_NUM   5
  #define ASIA_NUM   5
  #define AFRI_NUM   5
  #define AUST_NUM   5
  #define TEST_NUM   5
#elif defined(FULL_TREE)
  #define EURO_NUM   17
  #define ASIA_NUM   20
  #define AFRI_NUM   24
  #define AUST_NUM   38
  #define TEST_NUM    8
#else
  #error "Please define either TINY_TREE or FULL_TREE"
#endif


// custom led strip object / data type
struct Strip {
  const char* name;   // 
  int num_leds;       // number of leds in strip
  int milliamps;      // max mA as secondary safety measure
  CRGB* leds;         // points to the actual LED array
  int red_percent;    // state for basic fire effects
  // a bit advanced: creates pointer to a function
  void (*displayMode)(Strip&); 
  unsigned long last_effect_update; // state for timing
  int effect_delay;  // state for timing
  unsigned long satellite_interval_ms; 
};

// arrays of CRGB elements (red, green, blue), each 0-255
CRGB leds_europe[EURO_NUM];
CRGB leds_asia[ASIA_NUM];
CRGB leds_africa[AFRI_NUM];
CRGB leds_australia[AUST_NUM];
CRGB leds_test[TEST_NUM];

//MARK: forward declarations
// arduino doesn't care, cpp does! (relevant when using platformio)
// forward declarations (all valid displayMode functions -> can be directly assigned to strip)
void sarasLights(Strip& strip);
void regionFiresStatic(Strip& strip);
void regionFiresDynamic(Strip& strip);
void flickeringFire(Strip& strip);
void fancyGradient(Strip& strip);
void allLEDsOff(Strip& strip);  

void printStripStatus();
int totallyLegitSatelliteData(Strip& strip);

// example of display function which can be assigned to strips using wrappers 
// free to use whatever function interface you want! (though you probably want to include the strip)
void pulse(Strip& strip, uint16_t period_ms, CRGB color) {
  uint8_t brightness = beatsin8(60000 / period_ms, 0, 255);
  fill_solid(strip.leds, strip.num_leds, color);
  fadeToBlackBy(strip.leds, strip.num_leds, 255 - brightness);
}

// custom wrappers for each strip -> forward on to some other function
// basically a little trick to work around cpp limitation and allow assignment of functions that don't fit the required form inside the struct
// -> write your own function, then call it inside the region's wrappers you want it to work on, with whatever arguments you like
void europeDisplay(Strip& strip) {
  pulse(strip, 3000, CRGB::Orange);  // 3 second orange pulse
}

void asiaDisplay(Strip& strip) {
  regionFiresDynamic(strip);  
}

void africaDisplay(Strip& strip) {
  fancyGradient(strip);
}

void australiaDisplay(Strip& strip) {
  sarasLights(strip); 
}

void testDisplay(Strip& strip) {
  regionFiresDynamic(strip);
}

// instantiate strips in array for easy iterating over later
Strip strips[] = {
  {"Europe",    EURO_NUM,  800, leds_europe,    15, europeDisplay, 0, 0, 5*MINS}, 
  {"Asia",      ASIA_NUM, 1000, leds_asia,      30, asiaDisplay, 0, 0, 1*MINS},
  {"Africa",    AFRI_NUM, 1200, leds_africa,    40, africaDisplay, 0, 0, 5*MINS},
  {"Australia", AUST_NUM,  500, leds_australia, 23, australiaDisplay, 0, 0, 5*MINS},
  {"Test",      TEST_NUM,  200, leds_test,      20, testDisplay, 0, 0, 2000},
};
// auto-calculate number of strips 
// very easy to add or remove strips, then forget to manually change this
// so you no longer have to!
// gets size (in bytes) of all strips, then divides by size of single one (they're all of same size)
const int NUM_STRIPS = sizeof(strips) / sizeof(strips[0]); 

// some example colours used in fires display
CRGB colour_healthy = CRGB(195, 195, 10);
CRGB colour_fire = CRGB(255, 20, 0);

//MARK: setup
void setup() {
  Serial.begin(115200); 
  delay(1000); // give it some time to start

  Serial.println("=== Multi-Strip LED Controller ===");

  // init FastLED for each strip explicitly 
  // MUST be done like this (annoyingly, this can't easily be done via loop), hardcoding in pin and colour order
  pinMode(EURO_PIN, OUTPUT);
  FastLED.addLeds<WS2812B, EURO_PIN, EURO_ORDER>(strips[0].leds, strips[0].num_leds)
        .setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, strips[0].milliamps);
  
  pinMode(ASIA_PIN, OUTPUT);
  FastLED.addLeds<WS2812B, ASIA_PIN, ASIA_ORDER>(strips[1].leds, strips[1].num_leds)
        .setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, strips[1].milliamps);
  
  pinMode(AFRI_PIN, OUTPUT);
  FastLED.addLeds<WS2812B, AFRI_PIN, AFRI_ORDER>(strips[2].leds, strips[2].num_leds)
        .setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, strips[2].milliamps);
  
  pinMode(AUST_PIN, OUTPUT);
  FastLED.addLeds<WS2812B, AUST_PIN, AUST_ORDER>(strips[3].leds, strips[3].num_leds)
        .setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, strips[3].milliamps);

  pinMode(TEST_PIN, OUTPUT);
  FastLED.addLeds<WS2812B, TEST_PIN, TEST_ORDER>(strips[4].leds, strips[4].num_leds)
        .setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, strips[4].milliamps);
  
  // set brightness scaling for all strips, clear all leds
  FastLED.setBrightness(MAX_BRIGHTNESS);
  FastLED.clear();
  FastLED.show(); 

  // all done! 
  printStripStatus();
  Serial.println("Setup complete, running loop.");
}



//MARK: loop
void loop() {
  // using static to avoid having to use global variable 
  // -> basically turns it into a variable that persists between function calls (or iterations in this case)
  // (really just to keep this all in one place)
  static unsigned long last_update = 0;
  const int UPDATE_INTERVAL_MS = 50;  // 20 FPS
  unsigned long now_ms = millis();

  if (now_ms - last_update >= UPDATE_INTERVAL_MS) {
    last_update = now_ms;
    
    // update and render strips according to their selected display mode
    for (int i = 0; i < NUM_STRIPS; i++) {
      strips[i].displayMode(strips[i]);
    }
    
    FastLED.show(); // update all strips at once
  }
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

/* turn strip off */
void allLEDsOff(Strip& strip) {
  for (int i = 0; i < strip.num_leds; i++) {
    strip.leds[i] = CRGB(0, 0, 0);
  }
}

/* single colour for whole strip */
void singleColour(Strip& strip, CRGB color) {
  for (int i = 0; i < strip.num_leds; i++) {
    strip.leds[i] = color;
  }
}

/* basic gradient effect example */
void fancyGradient(Strip& strip) {
  int scalar = 12;
  for (int i = 0; i < strip.num_leds; i++) {
    if (i <= strip.num_leds / 2) {
      strip.leds[i] = CRGB(scalar*i, 160 - scalar*i, 0);
    } else {
      strip.leds[i] = CRGB(255, 64, 0);
    }
  }
}

/* flickering fire effect */
void flickeringFire(Strip& strip) {
  unsigned long now = millis();
  
  // random delay between updates for natural-ish flicker
  static int next_delay = random(40, 600);
  
  if (strip.effect_delay == 0) {  // first call
    strip.effect_delay = random(40, 600);
  }
    
  if (now - strip.last_effect_update >= strip.effect_delay) {
    strip.last_effect_update = now;
    strip.effect_delay = random(40, 600);
    
    for (int i = 0; i < strip.num_leds; i++) {
      int r = random(120, 255);
      int g = random(0, 60);
      strip.leds[i] = CRGB(r, g, 0);
    }
  }
}

/* 2/3 yellow-green, 1/3 red - Sara's original pattern */
void sarasLights(Strip& strip){ 
  int num_green = strip.num_leds * 2 / 3;

  // bottom 2/3: yellow-green (trunk)
  for (int i = 0; i < num_green; i++) {
    strip.leds[i] = CRGB(195, 195, 10);
  }

  // top 1/3: red (tip)
  for (int i = num_green; i < strip.num_leds; i++) {
    strip.leds[i] = CRGB(255, 20, 0);
  }
}

/* display region fires - static version */
void regionFiresStatic(Strip& strip) {
  int num_green = strip.num_leds * (100 - strip.red_percent) / 100;
  
  if (num_green < 0 || num_green > strip.num_leds) {
    Serial.print("ERROR: num_green out of bounds for ");
    Serial.println(strip.name);
    return;
  }

  for (int i = 0; i < num_green; i++) {
    strip.leds[i] = colour_healthy;
  }
  for (int i = num_green; i < strip.num_leds; i++) {
    strip.leds[i] = colour_fire;
  }
}

/* display region fires - using actual satellite data! (definitely) */
void regionFiresDynamic(Strip& strip) {
  unsigned long now = millis();
  
  // periodically retrieve satellite data
  if (now - strip.last_effect_update >= strip.satellite_interval_ms) {
    strip.last_effect_update = now;
    strip.red_percent = totallyLegitSatelliteData(strip);
  }
  
  // render using static update version
  regionFiresStatic(strip);
}


//MARK: helpers

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

// some nice functions to change display modes during runtime!
// either add some buttons or whatever, or via serial

/* change strip's display mode/function */
void setDisplayMode(int index, void (*newMode)(Strip&)) {
  if (index >= 0 && index < NUM_STRIPS) {
    strips[index].displayMode = newMode;
    Serial.print("Updated ");
    Serial.println(strips[index].name);
  }
}

/* Set all strips to same mode */
void setAllDisplayModes(void (*newMode)(Strip&)) {
  for (int i = 0; i < NUM_STRIPS; i++) {
    strips[i].displayMode = newMode;
  }
  Serial.println("All strip display functions updated");
}

// =====================================================================

//MARK: SECRET

/*
* ⋆⭒˚.⋆ 🌍 🛰️ ⋆⭒˚.⋆ TOP SECRET satellite data function ⋆⭒˚.⋆ 🪐🛸⋆⭒˚.⋆
* Somehow works without wlan! Some proper NASA-level engineering 👩🏻‍🔬
* - pass in region-led-strip 
* - returns percentage of fire [0-100]
*/
int totallyLegitSatelliteData(Strip& strip) {
  
  int current_red_percent = strip.red_percent;
  int change = random(-20, 21);  // magic!
  int new_red_percent = current_red_percent + change;
   
  // add some "gravity" to keep it roughly in reasonable range
  if (new_red_percent > 60) {
    new_red_percent -= random(3, 12);  // pull downward
  } else if (new_red_percent < 10) {
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
