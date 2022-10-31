//#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

// OLED FeatherWing buttons map to different pins depending on board:
#if defined(ESP8266)
  #define BUTTON_A  0
  #define BUTTON_B 16
  #define BUTTON_C  2
#elif defined(ESP32) && !defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2)
  #define BUTTON_A 15
  #define BUTTON_B 32
  #define BUTTON_C 14
#elif defined(ARDUINO_STM32_FEATHER)
  #define BUTTON_A PA15
  #define BUTTON_B PC7
  #define BUTTON_C PC5
#elif defined(TEENSYDUINO)
  #define BUTTON_A  4
  #define BUTTON_B  3
  #define BUTTON_C  8
#elif defined(ARDUINO_NRF52832_FEATHER)
  #define BUTTON_A 31
  #define BUTTON_B 30
  #define BUTTON_C 27
#else // 32u4, M0, M4, nrf52840, esp32-s2 and 328p
  #define BUTTON_A  9
  #define BUTTON_B  6
  #define BUTTON_C  5
#endif

uint8_t CannonPower = 50;
uint8_t maxPixelBrightness = 50;
uint8_t armIndex = 0;
uint8_t suitIndex = 0;
bool displaySleep = false;
bool rowSelected = false;
uint8_t activeRow = 3;

float cellVolts[3] = {3.3, 3.3, 3.3};
bool battConnected = false;

int mod( int x, int y ){
    return x<0 ? ((x+1)%y)+y-1 : x%y;
}

void HandleDisplayButtons()
{
  // figure out which interrupt was called. Recall these are pull-down switches
  if (digitalRead(BUTTON_A) == LOW)
  {
    // if in menu, this scrolls up
    if (!rowSelected)
    {
      activeRow = mod(activeRow-1, 3);
    }

    // Otherwise, this increases the value
    else
    {
      IncreaseRowValue(activeRow);
    }

    // debounce
    delay(100);
  }
  if (digitalRead(BUTTON_B) == LOW)
  {
    // if in menu, this selects the field
    if (!rowSelected)
    {
      rowSelected = true;
    }

    // Otherwise, saves value and returns to menu
    else
    {
      rowSelected = false;
      // Send data about changed system var to other arduino
      uint8_t buf[4];
      buf[0] = 0x30;
      buf[1] = CannonPower;
      buf[2] = maxPixelBrightness;
      buf[3] = suitIndex;
      Serial.write(buf, 4);
    }

    // debounce
    delay(100);
  }
  if (digitalRead(BUTTON_C) == LOW)
  {
    // if in menu, this scrolls up
    if (!rowSelected)
    {
      activeRow = mod(activeRow+1, 3);
    }

    // Otherwise, this increases the value
    else
    {
      DecreaseRowValue(activeRow);
    }

    // debounce
    delay(100);
  }
}


void IncreaseRowValue(int row)
{
  switch(row)
  {
    case 0: 
      if (CannonPower == 255) break;
      CannonPower += 5;
      break;
    case 1: if (maxPixelBrightness == 255) {break;} maxPixelBrightness += 5; break;
    case 2:  suitIndex = mod(suitIndex+1, 2); break;
  }
}

void DecreaseRowValue(int row)
{
  switch(row)
  {
    case 0: if (CannonPower == 0) {break;} CannonPower -= 5; break;
    case 1: if (maxPixelBrightness == 0) {break;} maxPixelBrightness = mod(maxPixelBrightness-5, 255); break;
    case 2:  suitIndex = mod(suitIndex-1, 2);break;
  }
}

void RenderDisplay()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0,0);

  for (int i = 0; i < 3; ++i)
  {
    if (i == activeRow)
    {
      if (rowSelected)
        display.print(">");
      else
        display.print("-");
    }
    else
    {
      display.print(" ");
    }

    switch(i)
    {
      case 0: display.print("Cannon: "); display.println(CannonPower); break;
      case 1: display.print("Brightness: "); display.println(maxPixelBrightness); break;
      case 2: 
      {
          display.print("Suit: ");
          if (suitIndex == 0)
            display.println("GREEN");
          else if (suitIndex == 1)
            display.println("GREEN PULSE");
          break;
      }
    }
  }

  display.println(" ");
  display.println(" cell voltages: ");
  display.print(" ");
  display.print(cellVolts[0], 1);
  display.print(" - ");
  display.print(cellVolts[1], 1);
  display.print(" - ");
  display.println(cellVolts[2], 1);

  if (cellVolts[0] < 2 || cellVolts[1] < 2 || cellVolts[2] < 2)
    display.println("Battery disconnected!");
  
  display.display(); 
 
}

void setup() {
  Serial.begin(19200);

  display.begin(0x3C, true); // Address 0x3C default

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  display.setRotation(1);

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
  pinMode(7, INPUT);

  // Clean out input buffer
  while (Serial.available() > 0) {
    Serial.read();
  }
}

void loop() {
  if (!displaySleep && digitalRead(7) == LOW)
  {
    displaySleep = true;

    // tell the other arduino to stop sending data
    Serial.write(0x58);
    
    display.fillScreen(0);
    display.display();
    delay(500);
  }
  else if (displaySleep && digitalRead(7) == HIGH)
  {
    displaySleep = false;
    // send message to other arduino to request data
    // This means send initial values
    
    Serial.write(0x52);
    delay(100);
    // data should have arrived by now. Read from 
  }

  if (!displaySleep)
  {
    if (Serial.available() >= 4)
    {
      uint8_t type = Serial.read();
      //float* voltages;
      switch (type)
      {
        case 0x56:
          uint8_t buf[12];
          for (int i = 0; i < 12; ++i)
            buf[i] = Serial.read();
          memcpy(cellVolts, buf, 12);
          //cellVolts[0] = voltages[0];
          //cellVolts[1] = voltages[1];
          //cellVolts[2] = voltages[2];
          break;
        case 0x41:
          // Expect 3 more bytes
          CannonPower = Serial.read();
          maxPixelBrightness = Serial.read();
          suitIndex = Serial.read() == 1? 1 : 0;
          break;
      }
    }
    HandleDisplayButtons();
    RenderDisplay();
  }
  else
  {
    delay(500);
  }
}
