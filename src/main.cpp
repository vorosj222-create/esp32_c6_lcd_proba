#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <U8g2_for_Adafruit_GFX.h>

// GPIO pinout
#define TFT_MOSI 6
#define TFT_SCLK 7
#define TFT_CS 14
#define TFT_DC 15
#define TFT_RST 21
#define TFT_BL 22 // background illumination

// Display
#define LCD_W 320
#define LCD_H 172
#define OFFSET_X 0
#define OFFSET_Y 34

// Display object
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// U8g2 adapter
U8G2_FOR_ADAFRUIT_GFX u8g2;

// Blinking timing
unsigned long lastBlinkTime = 0;
bool circleIsWhite = true;

void drawTestScreen()
{
   // Kék sáv (középen)
  tft.fillRect(0, 0, 320, 172, ST77XX_BLUE);

  u8g2.setForegroundColor(ST77XX_WHITE); // wrapper color
  u8g2.setBackgroundColor(ST77XX_BLUE);  

  // Példa font beállítása https://github.com/olikraus/u8g2/wiki/fntlist99
  //u8g2.setFont(u8g2_font_inb46_mf); // U8g2 font
  u8g2.setFont(u8g2_font_fub35_tf);
  // Kiírás U8g2 fonttal
  u8g2.setCursor(14, 50);
  u8g2.print("Hello world!");

  tft.setCursor(16, 70);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(4);
  tft.print("Hello world!");

  // draw the following objects to the bottom of the screen:
  // white filled circle (diameter 40px)
  tft.fillCircle(50, 140, 20, ST77XX_WHITE);
  
  // red filled triangle (40px height)
  tft.fillTriangle(160, 120, 200, 160, 120, 160, ST77XX_RED);
  
  // green rectangle (40x40)
  tft.fillRect(240, 120, 40, 40, ST77XX_GREEN);
}

void setup()
{
  Serial.begin(115200);

  // Background illumination ON
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  // SPI init (SCLK, MISO (-1), MOSI, CS)
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  // ST7789 init
  // Init(width, height) then turning, offset
  tft.init(LCD_H, LCD_W); // replace, because swap_xy = true volt

  // Color invert and rotation
  tft.invertDisplay(true);
  tft.setRotation(1); // landscape

  u8g2.begin(tft);

  tft.fillScreen(ST77XX_BLACK);

  // Test
  drawTestScreen();
}

void loop()
{
  // the white circle blinks white/purple every second
  unsigned long currentTime = millis();
  
  // Toggle every 1000ms (1 second)
  if (currentTime - lastBlinkTime >= 1000) {
    circleIsWhite = !circleIsWhite;
    lastBlinkTime = currentTime;
    
    // Redraw only the circle
    uint16_t circleColor = circleIsWhite ? ST77XX_WHITE : ST77XX_MAGENTA;
    tft.fillCircle(50, 140, 20, circleColor);
  }
}
