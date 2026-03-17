#include <FastLED.h>
#include "led_helpers.h"  // All the machinery lives here

// ============================================
// STUDENT CONFIGURATION AREA
// ============================================

// Your LED strip settings
#define MY_NUM_LEDS 20
#define MY_DATA_PIN 27
#define MY_COLOUR_ORDER GRB  // Try: RGB, GRB, or BRG if colors look wrong

CRGB my_leds[MY_NUM_LEDS];

// ============================================
// YOUR DISPLAY FUNCTION
// Change this to make your LEDs do different things!
// ============================================

void myDisplay(Strip& strip) {
  // Example: simple two-color pattern
  // Try changing the colors or using different functions!
  
  regionFiresStatic(strip);  // Shows static fire pattern
  
  // Other options you can try:
  // sarasLights(strip);
  // flickeringFire(strip);
  // gradient(strip, CRGB::Blue, CRGB::Purple);
  // pulse(strip, 2000, CRGB::Red);
}

// ============================================
// SETUP - Runs once at start
// ============================================

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Initialize your LED strip
  initStrip(my_leds, MY_NUM_LEDS, MY_DATA_PIN, MY_COLOUR_ORDER, myDisplay);
  
  Serial.println("LEDs ready! Edit myDisplay() to change the pattern.");
}

// ============================================
// LOOP - Runs forever
// ============================================

void loop() {
  updateLEDs();  // This handles all the timing and updates
  delay(50);
}