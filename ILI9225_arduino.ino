// Include application, user and local libraries
#include "SPI.h"
#include "TFT_22_ILI9225.h"

// Include font definition files
// NOTE: These files may not have all characters defined! Check the GFXfont def
// params 3 + 4, e.g. 0x20 = 32 = space to 0x7E = 126 = ~

#include <../fonts/FreeSans12pt7b.h>
#include <../fonts/FreeSans24pt7b.h>

#if defined (ARDUINO_ARCH_STM32F1)
#define TFT_RST PA1
#define TFT_RS  PA2
#define TFT_CS  PA0 // SS
#define TFT_SDI PA7 // MOSI
#define TFT_CLK PA5 // SCK
#define TFT_LED 0 // 0 if wired to +5V directly
#elif defined(ESP8266)
#define TFT_RST 4   // D2
#define TFT_RS  5   // D1
#define TFT_CLK 14  // D5 SCK
//#define TFT_SDO 12  // D6 MISO
#define TFT_SDI 13  // D7 MOSI
#define TFT_CS  15  // D8 SS
#define TFT_LED 2   // D4     set 0 if wired to +5V directly -> D3=0 is not possible !!
#elif defined(ESP32)
#define TFT_RST 26  // IO 26
#define TFT_RS  25  // IO 25
#define TFT_CLK 14  // HSPI-SCK
//#define TFT_SDO 12  // HSPI-MISO
#define TFT_SDI 13  // HSPI-MOSI
#define TFT_CS  15  // HSPI-SS0
#define TFT_LED 0   // 0 if wired to +5V directly
SPIClass hspi(HSPI);
#else
#define TFT_RST 8
#define TFT_RS  9
#define TFT_CS  10  // SS
#define TFT_SDI 11  // MOSI
#define TFT_CLK 13  // SCK
#define TFT_LED 3   // 0 if wired to +5V directly
#endif

#define TFT_BRIGHTNESS 200 // Initial brightness of TFT backlight (optional)

// Use hardware SPI (faster - on Uno: 13-SCK, 12-MISO, 11-MOSI)
TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED, TFT_BRIGHTNESS);
// Use software SPI (slower)
//TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK, TFT_LED, TFT_BRIGHTNESS);

// Variables and constants
int16_t x=0, y=0, w, h;

#define POT_PIN  A0

#define Right_indicator     2
#define Left_indicator      3
#define Headlight           4

#define BRAKE_PIN 5
#define LATCH_PIN 6

unsigned long previousMillis = 0;
const long interval = 100;  // Update every 100 milliseconds

void setup() {
  // put your setup code here, to run once:
#if defined(ESP32)
  hspi.begin();
  tft.begin(hspi);
#else
  tft.begin();
#endif
  tft.clear();

  tft.setOrientation(1);
  tft.setGFXFont(&FreeSans12pt7b);  // Set the font for text drawing

  // Set up the potentiometer pin as input
  pinMode(POT_PIN, INPUT);
  pinMode(Right_indicator, INPUT_PULLUP);
  pinMode(Left_indicator, INPUT_PULLUP);
  pinMode(Headlight, INPUT_PULLUP);
}

void drawIndicators(int y) {

  // Calculate positions for centered indicators
  int centerX = tft.maxX() / 2;
  int indicatorSpacing = 60; // Adjust as needed
  int indicator1X = centerX - indicatorSpacing;
  int indicator2X = centerX + indicatorSpacing;
  int headlampX = centerX;

  // Draw Indicator 1
  // int indicator1X = 160;
  if (digitalRead(Right_indicator) == LOW) {
    tft.fillCircle(indicator1X, y + 6, 10, COLOR_BLUE);
  } else {
    tft.drawCircle(indicator1X, y + 6, 10, COLOR_GRAY);
  }

  // Draw Indicator 2
  // int indicator2X = 180;
  if (digitalRead(Left_indicator) == LOW) {
    tft.fillCircle(indicator2X, y + 6, 10, COLOR_GREEN);
  } else {
    tft.drawCircle(indicator2X, y + 6, 10, COLOR_GRAY);
  }

  // Draw Headlamp Indicator
  // int headlampX = 200;
  if (digitalRead(Headlight) == LOW) {
    tft.fillTriangle(headlampX, y + 1, headlampX + 6, y + 11, headlampX - 6, y + 11, COLOR_YELLOW);
  } else {
    tft.drawTriangle(headlampX, y + 1, headlampX + 6, y + 11, headlampX - 6, y + 11, COLOR_GRAY);
  }
}

void drawBottomBoxes(int y) {
  // Calculate positions for centered boxes
  int centerX = tft.maxX() / 2;
  int boxSpacing = 20; // Adjust as needed
  int brakeBoxX = centerX - boxSpacing;
  int latchBoxX = centerX + boxSpacing;

  // Draw Brake Box
  if (digitalRead(BRAKE_PIN) == HIGH) {
    tft.fillRectangle(brakeBoxX - 10, y, 20, 20, COLOR_RED);
  } else {
    tft.drawRectangle(brakeBoxX - 10, y, 20, 20, COLOR_GRAY);
  }

  // Draw Latch Box
  if (digitalRead(LATCH_PIN) == HIGH) {
    tft.fillRectangle(latchBoxX - 10, y, 20, 20, COLOR_BLUE);
  } else {
    tft.drawRectangle(latchBoxX - 10, y, 20, 20, COLOR_GRAY);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long currentMillis = millis();

  // Read the potentiometer value every 100 milliseconds
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Read the potentiometer value
    int potValue = analogRead(POT_PIN);

    // Map the potentiometer value to the maximum progress bar width
    int maxProgressBarWidth = tft.maxX() - 2;
    int progressBarWidth = map(potValue, 0, 255, 0, maxProgressBarWidth);

    // Clear the previous progress bar, markings, and text (fill with black)
    tft.clear();

    // Calculate the y-coordinate for the new position of the progress bar
    int progressBarY = 40; // Adjust this value as needed

    // Draw the new progress bar with different colors based on ranges
    if (progressBarWidth < 100) {
      tft.drawRectangle(0, progressBarY, maxProgressBarWidth, 12, COLOR_ORANGE);
      tft.fillRectangle(1, progressBarY + 1, progressBarWidth, 10, COLOR_ORANGE);
    } else if (progressBarWidth >= 100 && progressBarWidth < 150) {
      tft.drawRectangle(0, progressBarY, maxProgressBarWidth, 12, COLOR_LIGHTGREEN);
      tft.fillRectangle(1, progressBarY + 1, progressBarWidth, 10, COLOR_LIGHTGREEN);
    } else if (progressBarWidth >= 150 && progressBarWidth < 200) {
      tft.drawRectangle(0, progressBarY, maxProgressBarWidth, 12, COLOR_TOMATO);
      tft.fillRectangle(1, progressBarY + 1, progressBarWidth, 10, COLOR_TOMATO);
    } else {
      tft.drawRectangle(0, progressBarY, maxProgressBarWidth, 12, COLOR_DARKRED);
      tft.fillRectangle(1, progressBarY + 1, progressBarWidth, 10, COLOR_DARKRED);
    }

    // Draw markings on the progress bar at intervals of 50
    for (int i = 50; i <= 255; i += 50) {
      int markingX = map(i, 0, 255, 0, maxProgressBarWidth);
      // Ensure the markings are within the bounds of the progress bar
      if (markingX < maxProgressBarWidth) {
        tft.drawRectangle(markingX, progressBarY + 1, markingX + 1, progressBarY + 11, COLOR_RED);
      }
    }

   


    // Calculate the y-coordinate for the new position of the text
    int textY = progressBarY + 25; // Adjust this value as needed

    // Draw the text "Speed" below the progress bar
    tft.drawGFXText(5, textY, "Speed", COLOR_WHITE);

     // Draw indicators
    drawIndicators(textY + 20);

        // Draw bottom boxes
    // drawBottomBoxes(tft.maxY() - 30);
  }
}
// void loop() {
//   // put your main code here, to run repeatedly:
//   // Read the potentiometer value
//   int potValue = analogRead(POT_PIN);

//   // Map the potentiometer value to the progress bar width
//   int maxProgressBarWidth = tft.maxX() - 2;
//   int progressBarWidth = map(potValue, 0, 255, 0, maxProgressBarWidth);

//   // Clear the previous progress bar (fill with black)
//   // tft.fillRectangle(1, 50, tft.maxX() - 2, 10, COLOR_BLACK);
  
//   tft.clear();

//   // Calculate the y-coordinate for the new position of the progress bar
//   int progressBarY = 20; // Adjust this value as needed

//   // Draw the new progress bar
//   // tft.drawRectangle(0, progressBarY, tft.maxX() - 1, 12, COLOR_WHITE);
//   // tft.fillRectangle(1, progressBarY + 1, progressBarWidth, 10, COLOR_GREEN);

//   // Draw the new progress bar with different colors based on ranges
//   if (progressBarWidth < 100) {
//     tft.drawRectangle(0, progressBarY, maxProgressBarWidth, 12, COLOR_ORANGE);
//     tft.fillRectangle(1, progressBarY + 1, progressBarWidth, 10, COLOR_ORANGE);
//   } else if (progressBarWidth >= 100 && progressBarWidth < 150) {
//     tft.drawRectangle(0, progressBarY, maxProgressBarWidth, 12, COLOR_LIGHTGREEN);
//     tft.fillRectangle(1, progressBarY + 1, progressBarWidth, 10, COLOR_LIGHTGREEN);
//   } else if (progressBarWidth >= 150 && progressBarWidth < 200) {
//     tft.drawRectangle(0, progressBarY, maxProgressBarWidth, 12, COLOR_TOMATO);
//     tft.fillRectangle(1, progressBarY + 1, progressBarWidth, 10, COLOR_TOMATO);
//   } else {
//     tft.drawRectangle(0, progressBarY, maxProgressBarWidth, 12, COLOR_DARKRED);
//     tft.fillRectangle(1, progressBarY + 1, progressBarWidth, 10, COLOR_DARKRED);
//   }


//   // Draw markings on the progress bar at intervals of 55
//   for (int i = 50; i <= 255; i += 50) {
//     int markingX = map(i, 0, 255, 0, tft.maxX() - 2);
//         // Ensure the markings are within the progress bar
//       // if (markingX < progressBarWidth) {
//         tft.drawRectangle(markingX, progressBarY + 1, markingX + 1, progressBarY + 11, COLOR_RED);
//       // }
//   }


// // Calculate the y-coordinate for the text based on the progress bar height
//   int textY = progressBarY + 23; 

//   // Draw the text "Speed" at the top
//   tft.drawGFXText(5, textY, "Speed", COLOR_WHITE);

//   delay(100); // Add a delay for stability
// }
