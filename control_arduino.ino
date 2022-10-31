#include <EEPROM.h>
#define CANNON_ADDR 0
#define LED_ADDR 1

#include <Adafruit_NeoPixel.h>
#define NUM_NEOPIXELS 68
#define NEOPIXEL_PIN 6
Adafruit_NeoPixel neopixels(NUM_NEOPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

#define BASE 0
#define CHARGING 1
#define FIRING 2

#define GREEN 1

uint8_t animation = BASE;
uint8_t suit = GREEN;
int animIteration = 0;
int animDelay = 80;

// LED pixel numbers on each ring
#define NUM_LEDS_PER_COL 17
uint8_t cols[4][NUM_LEDS_PER_COL] = {{ 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16},
                        {35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 17},
                        {36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 18},
                        {67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 19}};

// lookup tables for LED animations
#define BASE_ANIM_FRAMES 36
float baseV2[] = {0.1, 0.1, 0.1, 0.2, 0.3, 0.4, 0.8, 1, 1, 0.8, 0.4, 0.3, 0.2, 0.1, 
                  0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 
                  0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1};
float chargingValues[] = {0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.5, 1, 0.5};
float firingValues[] =   {0.8, 1, 0.8, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1};

float yellow[] = {255.0, 120.0, 0.0};

#include <Adafruit_TiCoServo.h>

Adafruit_TiCoServo trigger;
#define ZERO 2400
#define ONE_EIGHTY 750
#define NINETY 1500 

uint8_t CannonPower = 50;
uint8_t maxPixelBrightness = 50;
String armAnimation = "BASE";
String suitAnimation = "GREEN";
bool displaySleep = false;
 int currentCannonPower = 0;
int prevTrigVal;

bool screenAwake = false;

float cellVolts[3] = {3.3, 3.3, 3.3};

// Defines for battery voltage reader
#define K           0.00472199
#define CELLS       3
#define MAX_CELLS   3
#define BATT_CELL_1 A0
#define BATT_CELL_2 A1
#define BATT_CELL_3 A2

double cell_const[MAX_CELLS] = 
{
  1.0000,
  2.0000,
  3.1277
};

int battPins[MAX_CELLS] = 
{
  BATT_CELL_1,
  BATT_CELL_2,
  BATT_CELL_3
};

int mod( int x, int y ){
  return x<0 ? ((x+1)%y)+y-1 : x%y;
}

void ReadAndSendBatteryVoltage()
{
  // Populate battery state message.
  double prevVoltage = 0.0;
  for (int i = 0; i < CELLS; i++)
  {
    // Read raw voltage from analog pin.
    double cellVoltage = analogRead(battPins[i]) * K;

    // Scale reading to full voltage.
    cellVoltage *= cell_const[i];
    double tmp = cellVoltage;
    
    // Isolate current cell voltage.
    cellVoltage -= prevVoltage;
    prevVoltage = tmp;

    // Set current cell voltage to message.
    cellVolts[i] = (float)cellVoltage;
  }

  // send the data
  uint8_t buf[13];
  buf[0] = 0x56;
  memcpy(buf+1, (uint8_t*)cellVolts, 12);
  Serial.write(buf, 13);
}

void SendSystemVars()
{
  uint8_t buf[4];
  buf[0] = 0x41;
  buf[1] = CannonPower;
  buf[2] = maxPixelBrightness;
  buf[3] = suit;
  Serial.write(buf, 4);
}

void UpdateNeoPixelAnimation(int ms)
{
  if (animation == BASE)
  {
    for (int i = 0; i < NUM_LEDS_PER_COL; ++i)
    {
      float v = baseV2[mod(animIteration+i, BASE_ANIM_FRAMES)];
      int r = (int)(v * yellow[0]);
      int g = (int)(v * yellow[1]);
      int b = (int)(v * yellow[2]);
      neopixels.setPixelColor(cols[0][i], neopixels.Color(r, g, b));
      neopixels.setPixelColor(cols[1][i], neopixels.Color(r, g, b));
      neopixels.setPixelColor(cols[2][i], neopixels.Color(r, g, b));
      neopixels.setPixelColor(cols[3][i], neopixels.Color(r, g, b));
    }
    neopixels.show(); 
    delay(ms);
    animIteration = mod(animIteration - 1, BASE_ANIM_FRAMES);
  }
  else if (animation == CHARGING)
  {
    for (int i = 0; i < NUM_LEDS_PER_COL; ++i)
    {
      float v = chargingValues[mod(animIteration+i, NUM_LEDS_PER_COL)];
      int r = (int)(v * yellow[0]);
      int g = (int)(v * yellow[1]);
      int b = (int)(v * yellow[2]);
      neopixels.setPixelColor(cols[0][i], neopixels.Color(r, g, b));
      neopixels.setPixelColor(cols[1][i], neopixels.Color(r, g, b));
      neopixels.setPixelColor(cols[2][i], neopixels.Color(r, g, b));
      neopixels.setPixelColor(cols[3][i], neopixels.Color(r, g, b));
    }
    neopixels.show(); 
    delay(ms);
    animIteration = mod(animIteration + 1, NUM_LEDS_PER_COL);
  }
  else if (animation == FIRING)
  {
    for (int i = 0; i < NUM_LEDS_PER_COL; ++i)
    {
      float v = firingValues[mod(animIteration+i, NUM_LEDS_PER_COL)];
      int r = (int)(v * yellow[0]);
      int g = (int)(v * yellow[1]);
      int b = (int)(v * yellow[2]);
      neopixels.setPixelColor(cols[0][i], neopixels.Color(r, g, b));
      neopixels.setPixelColor(cols[1][i], neopixels.Color(r, g, b));
      neopixels.setPixelColor(cols[2][i], neopixels.Color(r, g, b));
      neopixels.setPixelColor(cols[3][i], neopixels.Color(r, g, b));
    }
    neopixels.show(); 
    delay(ms);
    animIteration = mod(animIteration - 1, NUM_LEDS_PER_COL);
  }
}

void fire(bool charging, bool firing)
{
  // on button pressed, spin up motors
  if (charging)
  {
    currentCannonPower = CannonPower;
    animation = CHARGING;
    animIteration = 0;
    animDelay = 10;
    analogWrite(5, currentCannonPower);
  }

  // on release,
  // rotate servo, power motors down, wind servo back
  else if (firing)
  {
    trigger.write(ONE_EIGHTY);
    currentCannonPower = 0;
    animation = FIRING;
    animIteration = 0;
    for (int i = 0; i < NUM_LEDS_PER_COL; ++i) 
    {
      // play the firing animation at high speed
      UpdateNeoPixelAnimation(15);
    }
    delay(30);
    neopixels.clear();
    neopixels.show();
    // let servo reach the 180 position
    delay (300);
    
    trigger.write(ZERO);
    animation = BASE;
    animDelay = 80;
    analogWrite(5, currentCannonPower);
  }
}

void setup() {
  Serial.begin(19200);

  for (byte i = 0; i < MAX_CELLS; ++i)
  {
    pinMode(battPins[i], INPUT);
  }

  // Read system vars from EEPROM
  CannonPower = EEPROM.read(0);
  maxPixelBrightness = EEPROM.read(1);

  // Set PWM pins
  // motor driver
  pinMode(5, OUTPUT);
  analogWrite(5, 0);

  // servo
  trigger.attach(10, 1000, 2000);
  trigger.write(ZERO); // -90

  // Trigger pin
  pinMode(12, INPUT);
  prevTrigVal = digitalRead(12);

  // neopixels
  neopixels.begin();
  neopixels.clear();
  neopixels.show(); 
  neopixels.setBrightness(maxPixelBrightness);
}

void loop() {
  
  // check for bytes on serial line
  if (Serial.available() > 0)
  {
    uint8_t msgType = Serial.read();
    if (msgType == 0x52)
    {
      screenAwake = true;
      // Send initial data
      SendSystemVars();
    }
    else if (msgType == 0x58)
    {
      screenAwake = false;
    }
    else if (msgType == 0x30)
    {
      // it's an update message.
      uint8_t buf[3];
      Serial.readBytes(buf, 3);
      CannonPower = buf[0];
      maxPixelBrightness = buf[1];
      suit = buf[2];
      // Save to EEPROM
      EEPROM.write(0, CannonPower);
      EEPROM.write(1, maxPixelBrightness);
      neopixels.setBrightness(maxPixelBrightness);
    }
  }

  // trigger pin
  // we need to check change on this one
  int curTrigVal = digitalRead(12);
  bool charging = (curTrigVal == HIGH && prevTrigVal == LOW);
  bool firing = (curTrigVal == LOW && prevTrigVal == HIGH);
  if (charging || firing)
  {
    fire(charging, firing);
    delay(100);
  }
  prevTrigVal = curTrigVal;

  // update the neopixel animation to the next frame
  // base animation is 50Hz
  UpdateNeoPixelAnimation(animDelay);
}
