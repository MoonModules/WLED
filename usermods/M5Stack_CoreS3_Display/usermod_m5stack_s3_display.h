#pragma once

#include "wled.h"
#include <SPI.h>
#include <Wire.h>

// LovyanGFX — no compile-time TFT_eSPI macros needed
#include <LovyanGFX.hpp>

#ifndef USERMOD_ID_M5STACK_CORE_S3_DISPLAY
  #define USERMOD_ID_M5STACK_CORE_S3_DISPLAY 95
#endif

// AXP2101 PMU I2C address
#define AXP2101_ADDR 0x34
// AW9523B GPIO Expander I2C address
#define AW9523B_ADDR 0x58

// M5Stack Core S3 — SPI3 (HSPI) on these pins
#define TFT_CS   3
#define TFT_DC  35
#define TFT_MOSI 37
#define TFT_SCLK 36

// LovyanGFX panel definition for ILI9342 on ESP32-S3
class LGFX_ILI9342_M5StackS3 : public lgfx::LGFX_Device {
private:
  lgfx::Bus_SPI       _bus_instance;
  lgfx::Panel_ILI9342 _panel_instance;

public:
  LGFX_ILI9342_M5StackS3(void) {
    // SPI bus configuration — ESP32-S3 HSPI (SPI3)
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host   = SPI3_HOST;
      cfg.spi_mode   = 0;
      cfg.freq_write = 10000000;
      cfg.freq_read  = 6000000;
      cfg.pin_mosi   = TFT_MOSI;
      cfg.pin_sclk   = TFT_SCLK;
      cfg.pin_miso   = -1;
      cfg.pin_dc     = TFT_DC;
      cfg.spi_3wire  = false;
      cfg.use_lock   = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    // Panel configuration
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs   = TFT_CS;
      cfg.pin_rst  = -1;       // managed externally (AW9523)
      cfg.bus_shared = false;
      cfg.panel_width  = 320;
      cfg.panel_height = 240;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.rgb_order = false;   // true=BGR (swap R/B), false=RGB
      cfg.memory_width  = 240;
      cfg.memory_height = 320;
      cfg.offset_rotation = 0; // 0=landscape (native 320x240), 1=+90°, 2=+180°
      cfg.invert = true;
      _panel_instance.config(cfg);
    }

    setPanel(&_panel_instance);
  }
};

static LGFX_ILI9342_M5StackS3 tft;


//class name. Use something descriptive and leave the ": public Usermod" part :)
class M5StackCoreS3DisplayUsermod : public Usermod {
  private:
    bool enabled = true;
    bool _initOk = false;

    // needRedraw marks if redraw is required to prevent often redrawing.
    bool needRedraw = true;
    // Next variables hold the previous known values to determine if redraw is required.
    String knownSsid = "";
    IPAddress knownIp;

    unsigned long lastUpdate = 0;

    // GEQ display constants
    static constexpr uint8_t NUM_GEQ_BANDS = 16;
    uint8_t HEADER_HEIGHT;  // Set dynamically in setup()
    uint8_t BAR_WIDTH;     // Set dynamically in setup()
    uint16_t prevBarHeight[NUM_GEQ_BANDS] = {0};  // Differential drawing: previous frame heights

    // Static vibrant color per bar based on position (rainbow: red → violet)
    uint16_t geqColor(uint8_t bandIndex) {
      switch (bandIndex % 16) {
        case  0: return 0xF800;  // red
        case  1: return 0xFC00;  // orange
        case  2: return 0xFC40;  // amber
        case  3: return 0xFFC0;  // yellow
        case  4: return 0x07E0;  // green
        case  5: return 0x07EF;  // green-cyan
        case  6: return 0x07FF;  // cyan
        case  7: return 0x001F;  // blue
        case  8: return 0x281F;  // blue-indigo
        case  9: return 0x481F;  // indigo
        case 10: return 0x701F;  // violet
        case 11: return 0x881F;  // violet-magenta
        case 12: return 0xF81F;  // magenta
        case 13: return 0xF81C;  // magenta-red
        case 14: return 0xF800;  // red
        case 15: return 0xF808;  // red-orange
        default: return 0xF800;
      }
    }

  public:
    //Functions called by WLED

    /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     */
    void setup()
    {
        Serial.println("M5StackS3Display: starting setup");

        // Ensure Wire is initialized (safe to call even if WLED already started it)
        if (i2c_sda >= 0 && i2c_scl >= 0) {
            Wire.begin(i2c_sda, i2c_scl);
        } else {
            Wire.begin();
        }
        Wire.setTimeout(50);

        // AXP2101 - enable DLDO1 (VCC_BL per user)
        // Read current state first
        Wire.beginTransmission(AXP2101_ADDR);
        Wire.write(0x90);
        Wire.endTransmission(false);
        Wire.requestFrom((uint8_t)AXP2101_ADDR, (uint8_t)1);
        byte pwr_orig = Wire.available() ? Wire.read() : 0x00;
        Serial.printf("M5StackS3Display: 0x90 orig: 0x%02X\n", pwr_orig);

        // Set DLDO1 voltage to 3.3V (voltage reg 0x99)
        // DLDO uses 100mV steps: (3300-500)/100 = 28 = 0x1C
        Wire.beginTransmission(AXP2101_ADDR);
        Wire.write(0x99);  // DLDO1 voltage register
        Wire.write(0x1C);  // 3.3V
        Wire.endTransmission();

        // Read DLDO1 voltage back
        Wire.beginTransmission(AXP2101_ADDR);
        Wire.write(0x99);
        Wire.endTransmission(false);
        Wire.requestFrom((uint8_t)AXP2101_ADDR, (uint8_t)1);
        byte dldo1_v = Wire.available() ? Wire.read() : 0x00;
        Serial.printf("M5StackS3Display: DLDO1 0x99: 0x%02X\n", dldo1_v);

        // Enable DLDO1 via bit 7 of 0x90
        byte pwr_new = pwr_orig | 0x80;  // set bit 7 (DLDO1 enable)
        Wire.beginTransmission(AXP2101_ADDR);
        Wire.write(0x90);
        Wire.write(pwr_new);
        Wire.endTransmission();

        delay(1);

        // Verify
        Wire.beginTransmission(AXP2101_ADDR);
        Wire.write(0x90);
        Wire.endTransmission(false);
        Wire.requestFrom((uint8_t)AXP2101_ADDR, (uint8_t)1);
        byte pwr_after = Wire.available() ? Wire.read() : 0x00;
        Serial.printf("M5StackS3Display: 0x90 after: 0x%02X\n", pwr_after);

        // AW9523 GPIO expander for LCD reset
        // P1_1 = LCD_RST
        Wire.beginTransmission(AW9523B_ADDR);
        Wire.write(0x00);  // GCR - push-pull for port 0
        Wire.write(0xFF);
        Wire.endTransmission();

        Wire.beginTransmission(AW9523B_ADDR);
        Wire.write(0x01);  // GCR - push-pull for port 1
        Wire.write(0xFF);
        Wire.endTransmission();

        Wire.beginTransmission(AW9523B_ADDR);
        Wire.write(0x07);  // P1 direction (0=output, 1=input)
        Wire.write(0xFD);  // P1_1 output, rest input
        Wire.endTransmission();

        // Toggle P1_1 for reset
        Wire.beginTransmission(AW9523B_ADDR);
        Wire.write(0x03);  // P1 output register
        Wire.write(0x00);  // P1_1 LOW (reset)
        Wire.endTransmission();
        delay(10);
        Wire.beginTransmission(AW9523B_ADDR);
        Wire.write(0x03);  // P1 output register
        Wire.write(0x02);  // P1_1 HIGH (release)
        Wire.endTransmission();
        delay(10);

        // Verify AW9523 P1 register access
        Wire.beginTransmission(AW9523B_ADDR);
        Wire.write(0x03);  // P1 output register
        Wire.endTransmission(false);
        Wire.requestFrom((uint8_t)AW9523B_ADDR, (uint8_t)1);
        byte p1_state = Wire.available() ? Wire.read() : 0x00;
        Serial.printf("M5StackS3Display: AW9523B P1 state: 0x%02X\n", p1_state);

        Serial.println("M5StackS3Display: init TFT");
        tft.init();

        // Lock SPI pins so they can't be reassigned to other usermods
        PinManagerPinType pins[] = {
            { (gpio_num_t)TFT_MOSI, true },
            { (gpio_num_t)TFT_SCLK, true },
            { (gpio_num_t)TFT_CS,   true },
            { (gpio_num_t)TFT_DC,  true }
        };
        if (!pinManager.allocateMultiplePins(pins, 4, PinOwner::UM_Unspecified)) {
            Serial.println("M5StackS3Display: SPI pin allocation FAILED — disabling usermod");
            enabled = false;
            _initOk = false;
            return;
        }
        Serial.println("M5StackS3Display: SPI pins allocated");

        Serial.printf("M5StackS3Display: TFT width: %d, height: %d\n", tft.width(), tft.height());

        // Dynamic header sizing
        tft.setTextSize(2);
        HEADER_HEIGHT = tft.fontHeight() + 10;  // 5px padding top and bottom
        BAR_WIDTH = tft.width() / NUM_GEQ_BANDS;  // 320 / 16 = 20

        // Show loading screen
        tft.fillScreen(TFT_BLACK);

        // Header background
        tft.fillRect(0, 0, tft.width(), HEADER_HEIGHT, TFT_DARKGREY);
        int16_t textY = (HEADER_HEIGHT - tft.fontHeight()) / 2;
        tft.setTextColor(TFT_WHITE);
        tft.setTextDatum(TL_DATUM);
        tft.drawString("WLED", 10, textY);
        tft.setTextDatum(TR_DATUM);
        tft.drawString("Loading...", tft.width() - 10, textY);

        Serial.println("M5StackS3Display: setup complete");
        _initOk = true;
    }

    /*
     * loop() is called continuously. Here you can check for events, read sensors, etc.
     */
    void loop() {
        if (!enabled || !_initOk) return;

        unsigned long now = millis();

        // Standard max framerate cap (~100 FPS, matches audio reactive update rate)
        if (now - lastUpdate < 10) return;

        // Fallback: if LEDs are hogging CPU, only block if we refreshed recently enough
        // Otherwise punch through to guarantee minimum 5 FPS on display
        if (strip.isUpdating() && (now - lastUpdate < 200)) {
            return;
        }

        lastUpdate = now;

        // Skip content/header updates while LEDs are being updated
        if (!strip.isUpdating()) {
            // Check if values which are shown on display changed from the last time.
            String currentSsid = apActive ? String(apSSID) : WiFi.SSID();
            IPAddress currentIp = apActive ? IPAddress(4, 3, 2, 1) : Network.localIP();

            if (currentSsid != knownSsid || currentIp != knownIp)
            {
                needRedraw = true;
                knownSsid = currentSsid;
                knownIp = currentIp;
            }

            // === HEADER: Only redraw when needed ===
            if (needRedraw) {
                // Gray background
                tft.fillRect(0, 0, tft.width(), HEADER_HEIGHT, TFT_DARKGREY);

                // Text vertically centered in header
                int16_t textY = (HEADER_HEIGHT - tft.fontHeight()) / 2;
                tft.setTextSize(2);
                tft.setTextColor(TFT_WHITE);

                // SSID on left
                tft.setTextDatum(TL_DATUM);
                tft.drawString(knownSsid.c_str(), 10, textY);

                // IP on right
                tft.setTextDatum(TR_DATUM);
                tft.drawString(knownIp.toString().c_str(), tft.width() - 10, textY);

                needRedraw = false;
            }
        }

        // === 16 BOUNCING BARS - differential drawing ===
        uint8_t geq[NUM_GEQ_BANDS];

        // Get audio data if available
        um_data_t *um_data = nullptr;
        if (usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)
            && um_data != nullptr
            && um_data->u_data[2] != nullptr) {
            // Real audio data
            memcpy(geq, um_data->u_data[2], NUM_GEQ_BANDS);
        } else {
            // Simulated bouncing bars
            unsigned long t = millis() / 100;
            for (uint8_t i = 0; i < NUM_GEQ_BANDS; i++) {
                geq[i] = ((t + i * 5) % 20) < 10
                    ? ((t + i * 5) % 20) * 25
                    : (20 - ((t + i * 5) % 20)) * 25;
            }
        }

        // Differential bar drawing - only touch pixels that changed
        uint16_t canvasBottom = tft.height() - 1;
        uint16_t availableHeight = tft.height() - HEADER_HEIGHT;

        for (uint8_t i = 0; i < NUM_GEQ_BANDS; i++) {
            uint16_t x = i * BAR_WIDTH;

            // barHeight: 0-255 maps to 0-availableHeight pixels
            uint16_t newBarHeight = (geq[i] * availableHeight) / 255;
            if (newBarHeight < 1) newBarHeight = 1;

            int16_t diff = newBarHeight - prevBarHeight[i];

            if (diff > 0) {
                // Bar grew: draw new colored pixels at bottom
                uint16_t bottomY = canvasBottom + 1;
                tft.fillRect(x, bottomY - newBarHeight, BAR_WIDTH - 1, diff, geqColor(i));
            } else if (diff < 0) {
                // Bar shrank: erase excess pixels starting from the OLD top down to the NEW top
                uint16_t oldTopY = canvasBottom + 1 - prevBarHeight[i];
                tft.fillRect(x, oldTopY, BAR_WIDTH - 1, -diff, TFT_BLACK);
            }
            // diff == 0: no change, skip entirely

            prevBarHeight[i] = newBarHeight;
        }
    }

    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     */
    void addToJsonInfo(JsonObject& root)
    {
      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

      JsonArray lightArr = user.createNestedArray("M5StackS3Display"); //name
      lightArr.add(enabled?F("installed"):F("disabled")); //unit
    }

    void addToJsonState(JsonObject& root) {
      root["M5S3_enabled"] = enabled;
    }
    void readFromJsonState(JsonObject& root) {
      enabled = root["M5S3_enabled"] | enabled;
    }

    /*
     * addToConfig() can be used to add custom persistent settings to the cfg.json file in the "um" (usermod) object.
     */
    void addToConfig(JsonObject& root)
    {
      JsonObject top = root.createNestedObject("M5StackS3Display");
      top["enabled"] = enabled;
    }

    void appendConfigData()
    {
      oappend(SET_F("addHB('M5StackS3Display');"));
    }

    /*
     * readFromConfig() is called BEFORE setup(). This is called by WLED when settings are loaded.
     */
    bool readFromConfig(JsonObject& root) {
      JsonObject top = root["M5StackS3Display"];
      if (!top.isNull()) enabled = top["enabled"] | true;
      return true;
    }

    /*
     * getId() allows you to optionally give your v2 usermod an unique ID (please define it in const.h!).
     */
    uint16_t getId()
    {
      return USERMOD_ID_M5STACK_CORE_S3_DISPLAY;
    }
};
