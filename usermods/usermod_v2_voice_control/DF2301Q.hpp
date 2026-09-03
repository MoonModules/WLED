/*!
 * @file DF2301Q.hpp
 * @brief I2C interface for DF2301Q voice recognition module
 * @note Uses Arduino Wire library for I2C communication
 */
#pragma once

#include <Wire.h>

#define DF2301Q_I2C_ADDR           0x64
#define DF2301Q_I2C_REG_CMDID      0x02
#define DF2301Q_I2C_REG_PLAY_CMDID 0x03
#define DF2301Q_I2C_REG_SET_MUTE   0x04
#define DF2301Q_I2C_REG_SET_VOLUME 0x05
#define DF2301Q_I2C_REG_WAKE_TIME  0x06

#define DF2301Q_POLL_INTERVAL_MS   100

class DF2301Q {
public:
  DF2301Q(uint8_t addr = DF2301Q_I2C_ADDR)
    : _addr(addr), _detected(false), _lastCmd(0), _failCount(0) { }

  // Check if device is present on I2C bus (with retry)
  bool detect(uint8_t retries = 3, uint16_t delayMs = 100) {
    for (uint8_t i = 0; i < retries; i++) {
      Wire.beginTransmission(_addr);
      uint8_t error = Wire.endTransmission();

      if (error == 0) {
        _detected = true;
        _failCount = 0;
        return true;
      }

      if (i < retries - 1) {
        delay(delayMs);
      }
    }
    _detected = false;
    return false;
  }

  // Quick check if module is still responding
  bool ping() {
    Wire.beginTransmission(_addr);
    return (Wire.endTransmission() == 0);
  }

  // Mark module as lost (called when communication fails repeatedly)
  void markLost() {
    _detected = false;
  }

  bool isDetected() const { return _detected; }

  // Poll for a voice command - call this from loop()
  // Returns command ID if a new command was detected, 0 otherwise
  uint8_t poll() {
    if (!_detected) return 0;

    uint8_t cmdID = 0;
    if (readReg(DF2301Q_I2C_REG_CMDID, &cmdID)) {
      _failCount = 0;  // Reset on successful read

      if (cmdID > 0 && cmdID != _lastCmd) {
        _lastCmd = cmdID;
        return cmdID;
      } else if (cmdID == 0) {
        // Reset lastCmd when no command pending, so same command can repeat
        _lastCmd = 0;
      }
    } else {
      // Track consecutive failures
      _failCount++;
      if (_failCount >= 10) {
        _detected = false;  // Mark as lost after 10 consecutive failures
      }
    }
    return 0;
  }

  uint8_t getFailCount() const { return _failCount; }

  void playByCMDID(uint8_t cmdID) {
    if (!_detected) return;
    writeReg(DF2301Q_I2C_REG_PLAY_CMDID, cmdID);
  }

  uint8_t getWakeTime() {
    if (!_detected) return 0;
    uint8_t time = 0;
    readReg(DF2301Q_I2C_REG_WAKE_TIME, &time);
    return time;
  }

  void setWakeTime(uint8_t time) {
    if (!_detected) return;
    writeReg(DF2301Q_I2C_REG_WAKE_TIME, time);
  }

  void setVolume(uint8_t vol) {
    if (!_detected) return;
    writeReg(DF2301Q_I2C_REG_SET_VOLUME, vol);
  }

  void setMute(bool mute) {
    if (!_detected) return;
    writeReg(DF2301Q_I2C_REG_SET_MUTE, mute ? 1 : 0);
  }

  uint8_t getLastCommand() const { return _lastCmd; }

private:
  bool writeReg(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(_addr);
    Wire.write(reg);
    Wire.write(value);
    return (Wire.endTransmission() == 0);
  }

  bool readReg(uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(_addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) {
      return false;
    }

    if (Wire.requestFrom(_addr, (uint8_t)1) != 1) {
      return false;
    }

    *value = Wire.read();
    return true;
  }

  uint8_t _addr;
  bool _detected;
  uint8_t _lastCmd;
  uint8_t _failCount;
};
