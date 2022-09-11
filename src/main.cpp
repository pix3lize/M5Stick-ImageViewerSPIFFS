#include <M5StickCPlus.h>

// Include the jpeg decoder library
#include <TJpg_Decoder.h>
// Include SPIFFS
#define FS_NO_GLOBALS
#include <FS.h>
#ifdef ESP32
#include "SPIFFS.h" // ESP32 only
#endif
#include "SPI.h"



// Support funtion prototypes
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);
void loadFile(const char *name);

//====================================================================================
//                                    Setup
//====================================================================================
void setup()
{
  // initialize the M5StickC object
  M5.begin();
  
  Serial.begin(115200);
  Serial.println("\n\n Testing TJpg_Decoder library");

  // Initialise SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS initialisation failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  Serial.println("\r\nInitialisation done.");

  // Initialise the TFT

  M5.Lcd.setTextColor(0xFFFF, 0x0000);
  M5.Lcd.fillScreen(TFT_BLACK);

  // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
  TJpgDec.setJpgScale(1);

  // The byte order can be swapped (set true for TFT_eSPI)
  TJpgDec.setSwapBytes(true);

  // The decoder must be given the exact name of the rendering function above
  TJpgDec.setCallback(tft_output);
}

//====================================================================================
//                                    Loop
//====================================================================================
#if defined(ESP32)
void loop()
{
  File root = SPIFFS.open("/");
  while (File file = root.openNextFile()) {
    String strname = file.name();
    // If it is not a directory and filename ends in .jpg then load it
    if (!file.isDirectory() && strname.endsWith(".jpg")) {
      loadFile(("/"+strname).c_str());
    }
  }
  M5.update();
}
#else   // ESP8266 has different SPIFFS methods
void loop()
{
  fs::Dir directory = SPIFFS.openDir("/");
  while (directory.next()) {
    String strname = directory.fileName();
    // If filename ends in .jpg then load it
    if (strname.endsWith(".jpg")) {
      loadFile(strname.c_str());
    }
  }
}
#endif

//====================================================================================
//                                    tft_output
//====================================================================================
// This next function will be called during decoding of the jpeg file to
// render each block to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  // Stop further decoding as image is running off bottom of screen
  if ( y >= M5.Lcd.height() ) return 0;

  // This function will clip the image block rendering automatically at the TFT boundaries
  M5.Lcd.pushImage(x, y, w, h, bitmap);

  // This might work instead if you adapt the sketch to use the Adafruit_GFX library
  // M5.Lcd.drawRGBBitmap(x, y, bitmap, w, h);

  // Return 1 to decode next block
  return 1;
}

//====================================================================================
//                                    load_file
//====================================================================================

void loadFile(const char *name)
{
  M5.Lcd.fillScreen(TFT_RED);

  // Time recorded for test purposes
  uint32_t t = millis();

  // Get the width and height in pixels of the jpeg if you wish
  uint16_t w = 0, h = 0, scale;
  TJpgDec.getFsJpgSize(&w, &h, name); // Note name preceded with "/"
  M5.Lcd.setRotation(w > h ? 1 : 0);

  for (scale = 1; scale <= 8; scale <<= 1) {
    if (w <= M5.Lcd.width() * scale && h <= M5.Lcd.height() * scale) break;
  }
  TJpgDec.setJpgScale(scale);

  // Draw the image, top left at 0,0
  TJpgDec.drawFsJpg(0, 0, name);

  // How much time did rendering take
  t = millis() - t;

  char buf[80];
  sprintf(buf, "%s %dx%d 1:%d %u ms", name, w, h, scale, t);
  M5.Lcd.setCursor(0, M5.Lcd.height() - 8);
  M5.Lcd.print(buf);
  Serial.println(buf);
  delay(2000);
}
