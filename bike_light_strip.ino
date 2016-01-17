/*

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Copyright John Cooper 2016


 */
#include "FastLED.h"

FASTLED_USING_NAMESPACE
// Drive some LED lights on my bike
// Default mode is white at the front, yellow at the sides
// and red at the back.

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    3
//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB

#define BRIGHTNESS         60
#define FRAMES_PER_SECOND  120

#define LEFT_START 6
#define FRONT_START LEFT_START + 8 // 6+8
#define RIGHT_START FRONT_START + 13 // 6+8+13
#define REAR_START RIGHT_START + 8  // 6+8+13+8
#define NUM_LEDS  REAR_START + 6 // 35 + 6
CRGB leds[NUM_LEDS];

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();

void drive_start();

void setup() {
  delay(3000); // 3 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  drive_start();
}


void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void addGlitter( fract8 chanceOfGlitter)
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}


void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13,0,NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16(i+7,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

CRGB driving_pos_colour(int pos) {
    if (pos < LEFT_START) {
        return CRGB::Red;
    }
    if (pos < FRONT_START) {
        return CRGB::Orange;
    }
    if (pos < RIGHT_START) {
        return CRGB::White;
    }
    if (pos < REAR_START) {
        return CRGB::Orange;
    }
    return CRGB::Red;
}

void drive_start() {
  // Turn on the driving lights
  // Set it all black to start
  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  for ( int i = 0; i < NUM_LEDS; i++) {
    FastLED.show();
    delay(200);
    leds[i] = driving_pos_colour(i);
    leds[NUM_LEDS - 1 - i ] = driving_pos_colour(i);
  }
}

int other_side(int pos) {
  return (NUM_LEDS - pos - 1);
}

void driving() {
  delay(200);
  for ( int i = FRONT_START - 1 ; i >= LEFT_START; i-- ) {
      leds[i] = CHSV(32, 32, 128);
      leds[other_side(i) ] = CHSV(32, 32, 32);
    if (i == (FRONT_START - 1)) {
      leds[LEFT_START] = CRGB::Orange;
      leds[REAR_START + 1] = CRGB::Orange;
    } else {
      leds[i+1] = CRGB::Orange;
      leds[other_side(i) - 1] = CRGB::Orange;
    }
    FastLED.show();
    delay(200);

  }

}

SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm, driving };
void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}


void loop()
{
  // Call the current pattern function once, updating the 'leds' array
  //gPatterns[gCurrentPatternNumber]();
  gPatterns[6]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND);

  // do some periodic updates
  //EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  //EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically

}
