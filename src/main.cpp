#include <Arduino.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// Beemeljük a háttérképet tartalmazó fájlt
#include "meter_background.h"

class LGFX_ESP32_C6 : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789  _panel_instance;
  lgfx::Bus_SPI       _bus_instance;

public:
  LGFX_ESP32_C6() {
    { // SPI busz beállítása
      auto cfg = _bus_instance.config();
      cfg.spi_host = SPI2_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.pin_sclk = 7;
      cfg.pin_mosi = 6;
      cfg.pin_miso = -1;
      cfg.pin_dc   = 15;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    { // Kijelző panel beállítása
      auto cfg = _panel_instance.config();
      cfg.pin_cs           = 14;    
      cfg.pin_rst          = 21;    
      cfg.pin_busy         = -1;
      cfg.panel_width      = 172;   
      cfg.panel_height     = 320;   
      cfg.offset_x         = 34;    // A bevált, pixelpontos vízszintes offset
      cfg.offset_y         = 0;     
      cfg.offset_rotation  = 0;     
      cfg.dummy_read_pixel = 8;
      cfg.readable         = false;
      cfg.invert           = true;  
      cfg.rgb_order        = false; 
      _panel_instance.config(cfg);
    }
    setPanel(&_panel_instance);
  }
};

LGFX_ESP32_C6 tft;

// A rajzlap (Sprite) a VU meter területéhez
LGFX_Sprite canvas(&tft);

#define TFT_BL 22 

// VU Meter geometriai állandók - FINOMÍTVA ÉS PONTOSÍTVA
const int METER_W = 172;
const int METER_H = 172;
const int CENTER_X = 86;    // Kijelző közepe vízszintesen
const int CENTER_Y = 120;   // A pontosított forgástengely magassága
const int NEEDLE_LEN = 84;  // A mutató teljes hossza a forgásponttól mérve

// Szöghatárok fokban mérve
const float MIN_ANGLE = 232.0; // Csönd / -20 dB
const float MAX_ANGLE = 308.0; // Csúcs / +5 dB

// Fizika és időzítés változói a finom mozgáshoz
float currentAngle = MIN_ANGLE;
float targetAngle = MIN_ANGLE;
unsigned long lastUpdateTime = 0;

// A VU meter dinamikus frissítése a háttér-pufferben (Dupla pufferelés)
void updateVUMeter() {
  // 1. Visszamásoljuk a tiszta, Nano Bananával retusált háttérképet a Flash-ből
  canvas.pushImage(0, 0, METER_W, METER_H, (const uint16_t*)vu_172x172_map);
  
  // Átváltjuk a szöget radiánba a trigonometriához
  float radians = currentAngle * DEG_TO_RAD;
  float cosRad = cos(radians);
  float sinRad = sin(radians);

  // 2. Kiszámoljuk a mutató belső KEZDŐPONTJÁT (a forgáspont felé eső egyharmadot kihagyjuk)
  int startLen = NEEDLE_LEN / 3; // Az első ~28 pixel láthatatlan marad
  int startX = CENTER_X + (int)(cosRad * startLen);
  int startY = CENTER_Y + (int)(sinRad * startLen);

  // 3. Kiszámoljuk a mutató külső VÉGPONTJÁT (a teljes hossznál)
  int endX = CENTER_X + (int)(cosRad * NEEDLE_LEN);
  int endY = CENTER_Y + (int)(sinRad * NEEDLE_LEN);
  
  // 4. Megrajzoljuk a lebegő fekete mutatót (2 pixel vastagon, körök nélkül)
  canvas.drawLine(startX, startY, endX, endY, TFT_BLACK);
  canvas.drawLine(startX + 1, startY, endX + 1, endY, TFT_BLACK);
  
  // 5. A kész kompozíciót (számlap + lebegő mutató) kitoljuk a kijelző tetejére
  canvas.pushSprite(0, 0);
}

// Az alsó rész statikus feliratainak egyszeri kirajzolása
void drawStaticInterface() {
  tft.fillRect(0, METER_H + 1, 172, 320 - (METER_H + 1), TFT_BLACK); // Alsó rész tisztítása
  
  tft.drawFastHLine(0, METER_H, 172, TFT_GREEN); // Zöld elválasztó vonal
  
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(10, 190);
  tft.print("INPUT: CH1 (I2S)");
  
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setCursor(10, 280);
  tft.print("SPRITE DMA ACTIVE");
}

void setup() {
  Serial.begin(115200);

  // Háttérvilágítás bekapcsolása
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  tft.init();
  tft.setRotation(0); // Álló tájolás (Portrait)
  tft.fillScreen(TFT_BLACK); 

  // Létrehozzuk a Sprite-ot a memóriában
  canvas.createSprite(METER_W, METER_H);
  canvas.setSwapBytes(true); // Biztosítja a megfelelő bájtsorrendet

  // Alsó feliratok kirajzolása
  drawStaticInterface();
}

void loop() {
  unsigned long currentTime = millis();
  
  // 20ms = 50 FPS képfrissítés a tükörsima animációhoz
  if (currentTime - lastUpdateTime >= 20) {
    lastUpdateTime = currentTime;

    // Szimulált zenei ugrálás: véletlenszerűen új célértéket választunk
    if (random(100) < 8) {
      targetAngle = random(MIN_ANGLE, MAX_ANGLE + 4); 
    }

    // Finom tehetetlenség/csillapítás (Ease physics): a mutató követi a célértéket
    // Felfelé dinamikusan ránt, lefelé finomabban esik vissza
    if (targetAngle > currentAngle) {
      currentAngle += (targetAngle - currentAngle) * 0.25;
    } else {
      currentAngle += (targetAngle - currentAngle) * 0.12;
    }

    // VU meter renderelése a kijelzőre
    updateVUMeter();

    // Élő érték kiírása dB-ben az alsó szekcióba
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(10, 215);
    tft.printf("VALUE: %02d dB ", (int)map(currentAngle, MIN_ANGLE, MAX_ANGLE, -20, 5));
  }
}
