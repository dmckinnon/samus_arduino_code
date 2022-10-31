// Most of this is the
// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// Released under the GPLv3 license to match the rest of the
// Adafruit NeoPixel library
// and then I've just modified what's necessary

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define PIN        6 // On Trinket or Gemma, suggest changing this to 1

#define NUM_CHEST_LEDS 14
#define CHEST_LOWER 0
#define CHEST_UPPER (CHEST_LOWER+NUM_CHEST_LEDS)

#define NUM_JETPACK_LEDS 12
#define JETPACK_LOWER (CHEST_UPPER)
#define JETPACK_UPPER (JETPACK_LOWER + NUM_JETPACK_LEDS)

#define NUM_THIGH_LEDS 58
#define LEFT_THIGH_LOWER (NUM_JETPACK_LEDS + NUM_CHEST_LEDS)
#define LEFT_THIGH_UPPER (LEFT_THIGH_LOWER+NUM_THIGH_LEDS)

#define NUM_SHIN_LEDS 48
#define LEFT_SHIN_LOWER (LEFT_THIGH_UPPER)
#define LEFT_SHIN_UPPER (LEFT_SHIN_LOWER+NUM_SHIN_LEDS)

#define RIGHT_THIGH_LOWER (LEFT_SHIN_UPPER)
#define RIGHT_THIGH_UPPER (RIGHT_THIGH_LOWER + NUM_THIGH_LEDS)

#define RIGHT_SHIN_LOWER (RIGHT_THIGH_UPPER)
#define RIGHT_SHIN_UPPER (RIGHT_SHIN_LOWER + NUM_SHIN_LEDS + 4)

#define SUIT_LED_BRIGHTNESS 60
#define JETPACK_LED_BRIGHTNESS 100

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS (NUM_CHEST_LEDS + NUM_JETPACK_LEDS + NUM_THIGH_LEDS + NUM_SHIN_LEDS + NUM_THIGH_LEDS + NUM_SHIN_LEDS+4)

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 1000 // Time (in milliseconds) to pause between pixels


void setup() {
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
}

void loop() {

   pixels.clear(); // Set all pixel colors to 'off'

  // suit leds loop
  for(int i = CHEST_LOWER; i < CHEST_UPPER; i++) {
    // GREEN
    pixels.setPixelColor(i, pixels.Color(0, SUIT_LED_BRIGHTNESS, 0));
  }

  for(int i = JETPACK_LOWER; i < JETPACK_UPPER; i++) {
    // BLUE
    pixels.setPixelColor(i, pixels.Color(0, 0, JETPACK_LED_BRIGHTNESS));
  }

  for(int i = LEFT_THIGH_LOWER; i < LEFT_THIGH_UPPER; i++) {
    // GREEN
    pixels.setPixelColor(i, pixels.Color(0, SUIT_LED_BRIGHTNESS, 0));
  }

  for(int i = LEFT_SHIN_LOWER; i < LEFT_SHIN_UPPER; i++) {
    // GREEN
    pixels.setPixelColor(i, pixels.Color(0, SUIT_LED_BRIGHTNESS, 0));
  }

  for(int i = RIGHT_THIGH_LOWER; i < RIGHT_THIGH_UPPER; i++) {
    // GREEN
    pixels.setPixelColor(i, pixels.Color(0, SUIT_LED_BRIGHTNESS, 0));
  }

  for(int i = RIGHT_SHIN_LOWER; i < RIGHT_SHIN_UPPER; i++) {
    // GREEN
    pixels.setPixelColor(i, pixels.Color(0, SUIT_LED_BRIGHTNESS, 0));
  }

  pixels.show();

   // Since we aren't pulsing, update every second
   delay(DELAYVAL);
}
