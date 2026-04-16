// #warning **** Included USERMOD_INA219 ****

#pragma once

#include <Arduino.h>   // WLEDMM: make sure that I2C drivers have the "right" Wire Object
#include "wled.h"
#include <Adafruit_INA219.h>

/*
 * Usermod for the INA219 I2C current/power sensor.
 *
 * Displays the following values in the Info tab:
 *   - Bus Voltage  (V)
 *   - Load Voltage (V)
 *   - Current      (mA)
 *   - Power        (mW)
 *
 * Configurable parameters (via WLED Settings > Usermods):
 *   - enabled            : enable/disable the usermod
 *   - i2cAddress         : I2C address (0x40, 0x41, 0x44, 0x45)
 *   - readInterval-ms    : measurement interval in milliseconds
 *   - shuntResistor-mOhm : shunt resistor value in milli-Ohm (default: 100 = 0.1 Ohm)
 *   - maxCurrentRange-A  : maximum expected current (0.4, 1.0, or 2.0 A) — selects PGA gain
 *   - busVoltageRange-V  : bus voltage range (16 or 32 V)
 *
 * Current and power are calculated directly from the shunt voltage:
 *   I = V_shunt / R_shunt  (independent of internal INA219 calibration)
 *
 * Requires: adafruit/Adafruit INA219 @ 1.2.1  (uncomment in platformio.ini)
 */

class UsermodINA219 : public Usermod {
private:
  Adafruit_INA219 *ina219 = nullptr;

  float shuntVoltage_mV = 0.0f;
  float busVoltage_V    = 0.0f;
  float current_mA      = 0.0f;
  float power_mW        = 0.0f;
  float loadVoltage_V   = 0.0f;

  bool sensorFound  = false;
  unsigned long lastMeasure = 0;

  // Configurable settings
  uint32_t readInterval       = 5000;   // ms between measurements
  uint8_t  i2cAddress         = 0x40;   // INA219 I2C address
  float    shuntResistor_mOhm = 100.0f; // shunt resistor in milli-Ohm (100 mOhm = 0.1 Ohm)
  float    maxCurrentRange_A  = 2.0f;   // max expected current: 0.4, 1.0, or 2.0 A
  uint8_t  busVoltageRange_V  = 32;     // bus voltage range: 16 or 32 V

  // PROGMEM string keys for config
  static const char _readInterval[];
  static const char _i2cAddress[];
  static const char _shuntResistor[];
  static const char _maxCurrentRange[];
  static const char _busVoltageRange[];

  // Select Adafruit calibration preset matching the configured voltage/current range.
  // This sets the correct PGA gain register in the INA219 Config register (BRNG + PG bits).
  // Current and power are still computed manually from the shunt voltage for accuracy.
  // Note: the Adafruit library has no 16V preset beyond 400 mA; for 16V + higher current
  // setCalibration_16V_400mA() is the only available 16V option and is used for all 16V cases.
  void applyCalibration() {
    if (!ina219) return;
    if (busVoltageRange_V <= 16) {
      ina219->setCalibration_16V_400mA();
    } else {
      if (maxCurrentRange_A <= 1.0f)
        ina219->setCalibration_32V_1A();
      else
        ina219->setCalibration_32V_2A();
    }
  }

public:
  UsermodINA219(const char *name, bool enabled) : Usermod(name, enabled) {}

  void setup() override {
    if (!enabled) return;

    if (!pinManager.joinWire()) {  // WLEDMM: allocates global I2C pins and starts Wire
      USER_PRINTLN(F("[INA219]: failed to join I2C bus."));
      sensorFound = false;
      initDone = true;
      return;
    }

    // Re-create sensor object with (potentially updated) I2C address
    if (ina219) { delete ina219; ina219 = nullptr; }
    ina219 = new Adafruit_INA219(i2cAddress);

    if (!ina219->begin()) {
      USER_PRINTLN(F("[INA219]: sensor not found."));
      delete ina219;
      ina219 = nullptr;
      sensorFound = false;
      initDone = true;
      return;
    }

    applyCalibration();
    sensorFound = true;
    USER_PRINTLN(F("[INA219]: sensor found."));
    initDone = true;
  }

  void loop() override {
    if (!enabled || !sensorFound || !initDone || !ina219) return;
    if (strip.isUpdating()) return;

    unsigned long now = millis();
    if (now - lastMeasure < readInterval) return;
    lastMeasure = now;

    // Read raw voltages directly from the sensor
    shuntVoltage_mV = ina219->getShuntVoltage_mV();
    busVoltage_V    = ina219->getBusVoltage_V();

    // Calculate load voltage, current and power manually using the configured shunt value.
    // This gives correct results for any shunt resistor, independent of the INA219 calibration.
    loadVoltage_V = busVoltage_V + (shuntVoltage_mV / 1000.0f);
    if (shuntResistor_mOhm < 1.0f) {
      // Guard against division by zero / near-zero shunt value (misconfigured)
      USER_PRINTLN(F("[INA219]: shuntResistor-mOhm is invalid (<1). Skipping current/power calculation."));
      current_mA = 0.0f;
      power_mW   = 0.0f;
    } else {
      current_mA = shuntVoltage_mV / (shuntResistor_mOhm / 1000.0f);  // I = U / R
      power_mW   = current_mA * loadVoltage_V;
    }
  }

  void addToJsonInfo(JsonObject &root) override {
    if (!enabled) return;

    JsonObject user = root[F("u")];
    if (user.isNull()) user = root.createNestedObject(F("u"));

    if (!initDone) {
      JsonArray arr = user.createNestedArray(F("INA219"));
      arr.add(F("Initializing..."));
      return;
    }

    if (!sensorFound) {
      JsonArray arr = user.createNestedArray(F("INA219"));
      arr.add(F("Not found"));
      return;
    }

    JsonArray busV  = user.createNestedArray(F("INA219 Bus Voltage"));
    busV.add(busVoltage_V);
    busV.add(F(" V"));

    JsonArray loadV = user.createNestedArray(F("INA219 Load Voltage"));
    loadV.add(loadVoltage_V);
    loadV.add(F(" V"));

    JsonArray curr  = user.createNestedArray(F("INA219 Current"));
    curr.add(current_mA);
    curr.add(F(" mA"));

    JsonArray pwr   = user.createNestedArray(F("INA219 Power"));
    pwr.add(power_mW);
    pwr.add(F(" mW"));
  }

  void addToConfig(JsonObject &root) override {
    JsonObject top = root.createNestedObject(FPSTR(_name));
    top[F("enabled")]                = enabled;
    top[FPSTR(_readInterval)]        = readInterval;
    top[FPSTR(_i2cAddress)]          = i2cAddress;
    top[FPSTR(_shuntResistor)]       = shuntResistor_mOhm;
    top[FPSTR(_maxCurrentRange)]     = maxCurrentRange_A;
    top[FPSTR(_busVoltageRange)]     = busVoltageRange_V;
    DEBUG_PRINTLN(F("[INA219] config saved."));
  }

  bool readFromConfig(JsonObject &root) override {
    JsonObject top = root[FPSTR(_name)];
    if (top.isNull()) {
      DEBUG_PRINTLN(F("[INA219]: No config found. (Using defaults.)"));
      return false;
    }
    bool configComplete = !top.isNull();

    uint8_t oldAddress = i2cAddress;

    configComplete &= getJsonValue(top[F("enabled")],            enabled,              false);
    configComplete &= getJsonValue(top[FPSTR(_readInterval)],    readInterval,        (uint32_t)5000);
    configComplete &= getJsonValue(top[FPSTR(_i2cAddress)],      i2cAddress,          (uint8_t)0x40);
    configComplete &= getJsonValue(top[FPSTR(_shuntResistor)],   shuntResistor_mOhm,  100.0f);
    if (shuntResistor_mOhm < 1.0f) {
      USER_PRINTLN(F("[INA219]: shuntResistor-mOhm clamped to minimum 1 mOhm."));
      shuntResistor_mOhm = 1.0f;
    }
    configComplete &= getJsonValue(top[FPSTR(_maxCurrentRange)], maxCurrentRange_A,   2.0f);
    configComplete &= getJsonValue(top[FPSTR(_busVoltageRange)], busVoltageRange_V,   (uint8_t)32);

    if (!initDone) {
      DEBUG_PRINTLN(F("[INA219] config loaded."));
    } else {
      DEBUG_PRINTLN(F("[INA219] config (re)loaded."));
      if (oldAddress != i2cAddress) {
        setup();  // reinitialize sensor with new I2C address
      } else {
        applyCalibration();  // update PGA gain for new voltage/current range
      }
    }
    return configComplete;
  }

  uint16_t getId() override { return USERMOD_ID_INA219; }
};

// PROGMEM string definitions
const char UsermodINA219::_readInterval[]    PROGMEM = "readInterval-ms";
const char UsermodINA219::_i2cAddress[]      PROGMEM = "i2cAddress";
const char UsermodINA219::_shuntResistor[]   PROGMEM = "shuntResistor-mOhm";
const char UsermodINA219::_maxCurrentRange[] PROGMEM = "maxCurrentRange-A";
const char UsermodINA219::_busVoltageRange[] PROGMEM = "busVoltageRange-V";
