#ifndef LED_HELPERS_H
#define LED_HELPERS_H

#include <FastLED.h>

// Strip structure (students don't need to understand this yet)
struct Strip {
  const char* name;
  int num_leds;
  int pin;
  int milliamps;
  EOrder color_order;
  CRGB* leds;
  int red_percent;
  void (*displayMode)(Strip&);
  unsigned long last_effect_update;
  int effect_delay;
  unsigned long satellite_interval_ms;
};

// Global strip (managed internally)
extern Strip myStrip;

// Easy initialization function
template<EOrder ORDER>
void initStrip(CRGB* leds, int num_leds, int pin, EOrder color_order, void (*displayFunc)(Strip&)) {
  myStrip = {"MyStrip", num_leds, pin, 1000, color_order, leds, 30, displayFunc, 0, 0, 5000};
  
  pinMode(pin, OUTPUT);
  
  if (ORDER == GRB) {
    FastLED.addLeds<WS2812B, pin, GRB>(leds, num_leds).setCorrection(TypicalLEDStrip);
  } else if (ORDER == BRG) {
    FastLED.addLeds<WS2812B, pin, BRG>(leds, num_leds).setCorrection(TypicalLEDStrip);
  } else {
    FastLED.addLeds<WS2812B, pin, RGB>(leds, num_leds).setCorrection(TypicalLEDStrip);
  }
  
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1000);
  FastLED.setBrightness(200);
  FastLED.clear();
  FastLED.show();
}

// Update function (call in loop)
void updateLEDs() {
  myStrip.displayMode(myStrip);
  FastLED.show();
}

// All your display functions
void sarasLights(Strip& strip);
void regionFiresStatic(Strip& strip);
void regionFiresDynamic(Strip& strip);
void flickeringFire(Strip& strip);
void gradient(Strip& strip, CRGB color1, CRGB color2);
void pulse(Strip& strip, uint16_t period_ms, CRGB color);
// ... etc

#endif