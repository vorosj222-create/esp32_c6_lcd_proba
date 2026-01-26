#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <U8g2_for_Adafruit_GFX.h>

// GPIO kiosztás az IDF kódod alapján
#define TFT_MOSI 6
#define TFT_SCLK 7
#define TFT_CS 14
#define TFT_DC 15
#define TFT_RST 21
#define TFT_BL 22 // Háttérvilágítás

// Kijelző paraméterei az IDF kódod alapján
#define LCD_W 320
#define LCD_H 172
#define OFFSET_X 0
#define OFFSET_Y 34

// Kijelző objektum létrehozása
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// U8g2 adapter példány
U8G2_FOR_ADAFRUIT_GFX u8g2;

void drawTestScreen()
{
  // Piros sáv (felül)
  tft.fillRect(0, 0, 320, 22, ST77XX_BLUE);

  // Kék sáv (középen)
  tft.fillRect(0, 22, 320, 64, ST77XX_BLUE);

  // Zöld sáv (alul)
  tft.fillRect(0, 86, 320, 86, ST77XX_BLUE);

  u8g2.setForegroundColor(ST77XX_WHITE); // wrapper saját színbeállítás
  u8g2.setBackgroundColor(ST77XX_BLUE);  // wrapper saját színbeállítás

  // Példa font beállítása https://github.com/olikraus/u8g2/wiki/fntlist99
  u8g2.setFont(u8g2_font_inb46_mf); // U8g2 font

  // Kiírás U8g2 fonttal
  u8g2.setCursor(10, 80);
  u8g2.print("7011.456");

  // Szöveg a kék sávba
  tft.setCursor(15, 45);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(6);
  // tft.print("7025.521");

  tft.setCursor(25, 110);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(4);
  tft.print("40m");

  tft.setCursor(240, 110);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(4);
  tft.print("CW");
}

void setup()
{
  Serial.begin(115200);

  // Háttérvilágítás bekapcsolása
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  // SPI inicializálása az egyedi lábakon (SCLK, MISO (-1), MOSI, CS)
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  // ST7789 inicializálása
  // Az init(szélesség, magasság) után manuálisan állítjuk be az elforgatást és az offsetet
  tft.init(LCD_H, LCD_W); // Felcseréljük, mert az IDF-ben swap_xy = true volt

  // Színek invertálása és elforgatás (az IDF esp_lcd_panel_invert_color alapján)
  tft.invertDisplay(true);
  tft.setRotation(1); // Fekvő mód

  u8g2.begin(tft);

  // A GAP (offset) kezelése: az Adafruit GFX-ben ezt néha manuálisan kell korrigálni,
  // ha a kijelző nem a 0,0 sarokból indul.
  // Az ST7789 könyvtárban ez az eltolás automatikusan alkalmazva lesz, ha jó típust választunk:
  tft.fillScreen(ST77XX_BLACK);

  // Teszt rajzolás
  drawTestScreen();
}

void loop()
{
  // Itt tarthatod a frissítéseket
}
