#pragma once

/*
   @title     MoonModules WLED - DF2301Q Voice Control Usermod
   @file      usermod_df2301q_voice.h
   @repo      https://github.com/MoonModules/WLED-MM, submit changes to this file as PRs to MoonModules/WLED-MM
   @Authors   TroyHacks, https://github.com/MoonModules/WLED-MM/commits/mdev/
   @Copyright © 2026 Github MoonModules Commit Authors (contact moonmodules@icloud.com for details)
   @license   Licensed under the EUPL-1.2 or later

   Voice recognition module for WLED using DFRobot DF2301Q
*/

#include "wled.h"
#include "DF2301Q.hpp"

class DF2301QUsermod : public Usermod {

private:

  bool initDone = false;
  DF2301Q* voiceModule = nullptr;
  unsigned long lastPollTime = 0;  // For polling interval timing

  // Configuration parameters
  uint8_t moduleVolume = 10;
  uint8_t wakeTime = 20;
  uint32_t pollInterval = DF2301Q_POLL_INTERVAL_MS;

  // Command mappings (0 = disabled)
  uint8_t cmdPowerOn = 103;      // "Turn on the light"
  uint8_t cmdPowerOff = 104;     // "Turn off the light"
  uint8_t cmdBrightnessUp = 105; // "Brighten the light"
  uint8_t cmdBrightnessDown = 106; // "Dim the light"
  uint8_t cmdNextPreset = 95;     // "The next track"
  uint8_t cmdPrevPreset = 94;     // "The last track"
  uint8_t cmdNextEffect = 0;     // Disabled by default
  uint8_t cmdPrevEffect = 0;     // Disabled by default

  // Feature toggles
  bool enableNumberedPresets = false;  // Commands 52-61 for presets 0-9
  bool enableColorCommands = false;    // Commands 116-123 for solid colors
  bool enableVoiceFeedback = false;    // Echo back recognized command
  uint8_t startupSound = 2;            // Command to play on startup (0 = disabled, 1 = "Yes, I'm here", 2 = "How can I help?")

  static const char _name[];
  static const char _enabled[];
  static const char _moduleVolume[];
  static const char _wakeTime[];
  static const char _pollInterval[];
  static const char _cmdPowerOn[];
  static const char _cmdPowerOff[];
  static const char _cmdBrightnessUp[];
  static const char _cmdBrightnessDown[];
  static const char _cmdNextPreset[];
  static const char _cmdPrevPreset[];
  static const char _cmdNextEffect[];
  static const char _cmdPrevEffect[];
  static const char _enableNumberedPresets[];
  static const char _enableColorCommands[];
  static const char _enableVoiceFeedback[];
  static const char _startupSound[];

public:

  DF2301QUsermod(bool en = false) : Usermod("DF2301Q", en) {
    // Constructor
  }

  ~DF2301QUsermod() {
    if (voiceModule) {
      delete voiceModule;
      voiceModule = nullptr;
    }
  }

  void setup() {
    if (!enabled) return;

    USER_PRINT(F("DF2301Q Voice Control startup; enabled = "));
    USER_PRINT(enabled ? F("true") : F("false"));
    USER_PRINTLN(F("."));

    // Check that global I2C pins are valid
    if (i2c_sda < 0 || i2c_scl < 0) {
      USER_PRINTLN(F("DF2301Q: Global I2C pins not configured"));
      return;
    }

    // Join the global I2C bus
    if (!pinManager.joinWire()) {
      USER_PRINTLN(F("DF2301Q: Failed to join I2C bus"));
      return;
    }

    // Create the voice module instance using global I2C bus
    voiceModule = new DF2301Q(DF2301Q_I2C_ADDR);

    // Detect and initialize the voice module
    if (voiceModule->detect()) {
      USER_PRINTLN(F("DF2301Q: Module detected!"));

      // Configure the module
      voiceModule->setVolume(moduleVolume);
      voiceModule->setWakeTime(wakeTime);

      // Play startup sound if configured
      if (startupSound > 0) {
        voiceModule->playByCMDID(startupSound);
        USER_PRINTF("DF2301Q: Played startup sound %d\n", startupSound);
      }
    } else {
      USER_PRINTLN(F("DF2301Q: Module not found on I2C bus - will retry in loop()"));
    }

    initDone = true;
  }

  void connected() {
    // Called when WiFi connects - not needed for this usermod
  }

  void loop() {
    if (!enabled || !voiceModule) return;

    // Check if module was lost and try to recover
    if (!voiceModule->isDetected()) {
      // Only try recovery every 5 seconds
      static unsigned long lastRecoveryAttempt = 0;
      if (millis() - lastRecoveryAttempt > 5000) {
        lastRecoveryAttempt = millis();
        USER_PRINTLN(F("DF2301Q: Attempting to reconnect..."));
        if (voiceModule->detect(3, 100)) {
          USER_PRINTLN(F("DF2301Q: Module reconnected!"));
          voiceModule->setVolume(moduleVolume);
          voiceModule->setWakeTime(wakeTime);
        }
      }
      return;
    }

    // Poll for voice commands at configured interval
    unsigned long now = millis();
    if (now - lastPollTime >= pollInterval) {
      lastPollTime = now;

      uint8_t cmdID = voiceModule->poll();
      if (cmdID > 0) {
        handleVoiceCommand(cmdID);
      }
    }
  }

  void addToJsonInfo(JsonObject& root) {
    JsonObject user = root["u"];
    if (user.isNull()) {
      user = root.createNestedObject("u");
    }

    if (!enabled) return;

    String uiNameString = FPSTR(_name);
    if (voiceModule && voiceModule->isDetected()) {
      uiNameString += F(" Detected");
    } else {
      uiNameString += F(" Not Found");
    }

    JsonArray infoArr = user.createNestedArray(uiNameString);

    String uiDomString;
    if (voiceModule && voiceModule->isDetected()) {
      uiDomString = F("Last Command: ");
      uiDomString += String(voiceModule->getLastCommand());
      uiDomString += F("<br />Volume: ");
      uiDomString += String(moduleVolume);
      uiDomString += F("<br />Wake Time: ");
      uiDomString += String(wakeTime);
      uiDomString += F("s");
    } else {
      uiDomString = F("Module not detected");
    }

    infoArr.add(uiDomString);
  }

  void readFromJsonState(JsonObject& root) {
    // Not used for this usermod
  }

  void appendConfigData() {
    oappend(SET_F("addHB('DF2301Q');"));

    // Define options array once, reuse for all dropdowns
    // Format: [value, "label"] - labels show actual DF2301Q voice phrases
    oappend(SET_F(
      "var dfo=["
        "[0,'Disabled'],"
        "[1,'Hello Robot (Wake)'],"
        "[2,'Custom Wake Word'],"
        "[5,'Custom Cmd 1'],[6,'Custom Cmd 2'],[7,'Custom Cmd 3'],[8,'Custom Cmd 4'],"
        "[9,'Custom Cmd 5'],[10,'Custom Cmd 6'],[11,'Custom Cmd 7'],[12,'Custom Cmd 8'],"
        "[13,'Custom Cmd 9'],[14,'Custom Cmd 10'],[15,'Custom Cmd 11'],[16,'Custom Cmd 12'],"
        "[17,'Custom Cmd 13'],[18,'Custom Cmd 14'],[19,'Custom Cmd 15'],[20,'Custom Cmd 16'],[21,'Custom Cmd 17'],"
        "[52,'Display number zero'],[53,'Display number one'],[54,'Display number two'],"
        "[55,'Display number three'],[56,'Display number four'],[57,'Display number five'],"
        "[58,'Display number six'],[59,'Display number seven'],[60,'Display number eight'],[61,'Display number nine'],"
        "[82,'Reset'],"
        "[92,'Play music'],[93,'Stop playing'],[94,'The last track'],[95,'The next track'],"
        "[103,'Turn on the light'],"
        "[104,'Turn off the light'],"
        "[105,'Brighten the light'],"
        "[106,'Dim the light'],"
        "[107,'Adjust brightness to maximum'],"
        "[108,'Adjust brightness to minimum'],"
        "[109,'Increase color temperature'],"
        "[110,'Decrease color temperature'],"
        "[113,'Daylight mode'],"
        "[114,'Moonlight mode'],"
        "[115,'Color mode'],"
        "[116,'Set to Red'],"
        "[117,'Set to Orange'],"
        "[118,'Set to Yellow'],"
        "[119,'Set to Green'],"
        "[120,'Set to Cyan'],"
        "[121,'Set to Blue'],"
        "[122,'Set to Purple'],"
        "[123,'Set to White']"
      "];"
      "function dfD(f){"
        "var dd=addDropdown('DF2301Q',f);"
        "for(var i=0;i<dfo.length;i++)addOption(dd,dfo[i][1],dfo[i][0]);"
      "}"
    ));

    // Create dropdowns using the shared options
    oappend(SET_F("dfD('cmd_Power_On');"));
    oappend(SET_F("dfD('cmd_Power_Off');"));
    oappend(SET_F("dfD('cmd_Brightness_Up');"));
    oappend(SET_F("dfD('cmd_Brightness_Down');"));
    oappend(SET_F("dfD('cmd_Next_Preset');"));
    oappend(SET_F("dfD('cmd_Previous_Preset');"));
    oappend(SET_F("dfD('cmd_Next_Effect');"));
    oappend(SET_F("dfD('cmd_Previous_Effect');"));
    // Startup_Sound left as number input for testing different command IDs

    // Volume dropdown (0-20, higher values may distort on some speakers)
    oappend(SET_F("dd=addDropdown('DF2301Q','Volume');"));
    oappend(SET_F("for(var i=0;i<=20;i++)addOption(dd,i,i);"));

    // Hints for settings
    oappend(SET_F("addInfo('DF2301Q:Poll_Interval',1,'ms between voice checks');"));
    oappend(SET_F("addInfo('DF2301Q:Wake_Time',1,'seconds module listens after wake word');"));
    oappend(SET_F("addInfo('DF2301Q:Startup_Sound',1,'phrase spoken when WLED starts');"));
    oappend(SET_F("addInfo('DF2301Q:Numbered_Presets',1,'\"Display number [1-9]\" → Preset 1-9, \"zero\" → Preset 10');"));
    oappend(SET_F("addInfo('DF2301Q:Color_Commands',1,'\"Set to Red/Orange/Yellow/Green/Cyan/Blue/Purple/White\"');"));

    // Usage instructions
    oappend(SET_F("addInfo('DF2301Q:Voice_Feedback',1,"
      "'<br><br><b>Usage:</b> Say \"Hello Robot\" (or your custom wake-phrase) to wake, then give commands.<br />"
      "Module listens for Wake Time seconds.<br />Each command resets timer so you can say multiple commands.');"));
  }

  void addToConfig(JsonObject& root) {
    JsonObject top = root.createNestedObject(FPSTR(_name));

    top[FPSTR(_enabled)] = enabled;
    top[FPSTR(_moduleVolume)] = moduleVolume;
    top[FPSTR(_wakeTime)] = wakeTime;
    top[FPSTR(_pollInterval)] = pollInterval;
    top[FPSTR(_cmdPowerOn)] = cmdPowerOn;
    top[FPSTR(_cmdPowerOff)] = cmdPowerOff;
    top[FPSTR(_cmdBrightnessUp)] = cmdBrightnessUp;
    top[FPSTR(_cmdBrightnessDown)] = cmdBrightnessDown;
    top[FPSTR(_cmdNextPreset)] = cmdNextPreset;
    top[FPSTR(_cmdPrevPreset)] = cmdPrevPreset;
    top[FPSTR(_cmdNextEffect)] = cmdNextEffect;
    top[FPSTR(_cmdPrevEffect)] = cmdPrevEffect;
    top[FPSTR(_enableNumberedPresets)] = enableNumberedPresets;
    top[FPSTR(_enableColorCommands)] = enableColorCommands;
    top[FPSTR(_enableVoiceFeedback)] = enableVoiceFeedback;
    top[FPSTR(_startupSound)] = startupSound;

    USER_PRINTLN(F("DF2301Q: Config saved."));
  }

  bool readFromConfig(JsonObject& root) {
    JsonObject top = root[FPSTR(_name)];

    if (top.isNull()) {
      USER_PRINT(FPSTR(_name));
      USER_PRINTLN(F(": No config found. (Using defaults.)"));
      return false;
    }

    enabled = top[FPSTR(_enabled)] | enabled;
    moduleVolume = top[FPSTR(_moduleVolume)] | moduleVolume;
    moduleVolume = constrain(moduleVolume, 0, 20);  // 0-20, higher values may distort
    wakeTime = top[FPSTR(_wakeTime)] | wakeTime;
    pollInterval = top[FPSTR(_pollInterval)] | pollInterval;
    cmdPowerOn = top[FPSTR(_cmdPowerOn)] | cmdPowerOn;
    cmdPowerOff = top[FPSTR(_cmdPowerOff)] | cmdPowerOff;
    cmdBrightnessUp = top[FPSTR(_cmdBrightnessUp)] | cmdBrightnessUp;
    cmdBrightnessDown = top[FPSTR(_cmdBrightnessDown)] | cmdBrightnessDown;
    cmdNextPreset = top[FPSTR(_cmdNextPreset)] | cmdNextPreset;
    cmdPrevPreset = top[FPSTR(_cmdPrevPreset)] | cmdPrevPreset;
    cmdNextEffect = top[FPSTR(_cmdNextEffect)] | cmdNextEffect;
    cmdPrevEffect = top[FPSTR(_cmdPrevEffect)] | cmdPrevEffect;
    enableNumberedPresets = top[FPSTR(_enableNumberedPresets)] | enableNumberedPresets;
    enableColorCommands = top[FPSTR(_enableColorCommands)] | enableColorCommands;
    enableVoiceFeedback = top[FPSTR(_enableVoiceFeedback)] | enableVoiceFeedback;
    startupSound = top[FPSTR(_startupSound)] | startupSound;

    // Apply settings to module if already initialized
    if (initDone && voiceModule && voiceModule->isDetected()) {
      voiceModule->setVolume(moduleVolume);
      voiceModule->setWakeTime(wakeTime);
    }

    USER_PRINT(FPSTR(_name));
    USER_PRINTLN(F(" config (re)loaded."));

    return true;
  }

  uint16_t getId() {
    return USERMOD_ID_VOICE_CONTROL; // You'll need to add this to const.h
  }

private:

  void handleVoiceCommand(uint8_t cmdID) {
    // Log wake words with friendly names
    if (cmdID == 1) {
      USER_PRINTLN(F("DF2301Q: Default Wake-Word detected"));
    } else if (cmdID == 2) {
      USER_PRINTLN(F("DF2301Q: Custom Wake-Word detected"));
    } else {
      USER_PRINTF("DF2301Q: Command received: %d\n", cmdID);
    }

    // Play back the recognized command phrase if enabled
    if (enableVoiceFeedback && voiceModule) {
      voiceModule->playByCMDID(cmdID);
    }

    // Check mapped commands (skip if mapped to 0/disabled)
    if (cmdPowerOn > 0 && cmdID == cmdPowerOn) {
      bri = briLast;
      stateUpdated(CALL_MODE_BUTTON);
      USER_PRINTLN(F("DF2301Q: Power ON"));

    } else if (cmdPowerOff > 0 && cmdID == cmdPowerOff) {
      briLast = bri;
      bri = 0;
      stateUpdated(CALL_MODE_BUTTON);
      USER_PRINTLN(F("DF2301Q: Power OFF"));

    } else if (cmdBrightnessUp > 0 && cmdID == cmdBrightnessUp) {
      bri = min(255, (int)bri + 25);
      stateUpdated(CALL_MODE_BUTTON);
      USER_PRINTF("DF2301Q: Brightness UP to %d\n", bri);

    } else if (cmdBrightnessDown > 0 && cmdID == cmdBrightnessDown) {
      bri = max(0, (int)bri - 25);
      stateUpdated(CALL_MODE_BUTTON);
      USER_PRINTF("DF2301Q: Brightness DOWN to %d\n", bri);

    } else if (cmdNextPreset > 0 && cmdID == cmdNextPreset) {
      applyPreset(currentPreset + 1, CALL_MODE_BUTTON);
      USER_PRINTLN(F("DF2301Q: Next Preset"));

    } else if (cmdPrevPreset > 0 && cmdID == cmdPrevPreset) {
      if (currentPreset > 0) {
        applyPreset(currentPreset - 1, CALL_MODE_BUTTON);
      }
      USER_PRINTLN(F("DF2301Q: Previous Preset"));

    } else if (cmdNextEffect > 0 && cmdID == cmdNextEffect) {
      strip.setMode(strip.getMainSegmentId(), (strip.getMainSegment().mode + 1) % strip.getModeCount());
      stateUpdated(CALL_MODE_BUTTON);
      USER_PRINTLN(F("DF2301Q: Next Effect"));

    } else if (cmdPrevEffect > 0 && cmdID == cmdPrevEffect) {
      uint16_t mode = strip.getMainSegment().mode;
      strip.setMode(strip.getMainSegmentId(), mode > 0 ? mode - 1 : strip.getModeCount() - 1);
      stateUpdated(CALL_MODE_BUTTON);
      USER_PRINTLN(F("DF2301Q: Previous Effect"));

    // Numbered presets: commands 52-61 ("Display number zero" through "nine") map to presets 1-10
    // "zero" maps to preset 10, "one" through "nine" map to presets 1-9
    } else if (enableNumberedPresets && cmdID >= 52 && cmdID <= 61) {
      uint8_t presetNum = (cmdID == 52) ? 10 : (cmdID - 52);
      applyPreset(presetNum, CALL_MODE_BUTTON);
      USER_PRINTF("DF2301Q: Apply Preset %d\n", presetNum);

    // Color commands: 116-123
    } else if (enableColorCommands && cmdID >= 116 && cmdID <= 123) {
      uint32_t color = 0;
      const char* colorName = "";
      switch (cmdID) {
        case 116: color = 0xFF0000; colorName = "Red"; break;
        case 117: color = 0xFF8000; colorName = "Orange"; break;
        case 118: color = 0xFFFF00; colorName = "Yellow"; break;
        case 119: color = 0x00FF00; colorName = "Green"; break;
        case 120: color = 0x00FFFF; colorName = "Cyan"; break;
        case 121: color = 0x0000FF; colorName = "Blue"; break;
        case 122: color = 0x8000FF; colorName = "Purple"; break;
        case 123: color = 0xFFFFFF; colorName = "White"; break;
      }
      // Set to solid color effect and apply color
      strip.setMode(strip.getMainSegmentId(), FX_MODE_STATIC);
      strip.getMainSegment().setColor(0, color);
      stateUpdated(CALL_MODE_BUTTON);
      USER_PRINTF("DF2301Q: Set color to %s\n", colorName);

    } else {
      USER_PRINTF("DF2301Q: Unknown command: %d\n", cmdID);
    }
  }
};

// Config variable names
const char DF2301QUsermod::_name[]              PROGMEM = "DF2301Q";
const char DF2301QUsermod::_enabled[]           PROGMEM = "Enabled";
const char DF2301QUsermod::_moduleVolume[]      PROGMEM = "Volume";
const char DF2301QUsermod::_wakeTime[]          PROGMEM = "Wake_Time";
const char DF2301QUsermod::_pollInterval[]      PROGMEM = "Poll_Interval";
const char DF2301QUsermod::_cmdPowerOn[]        PROGMEM = "cmd_Power_On";
const char DF2301QUsermod::_cmdPowerOff[]       PROGMEM = "cmd_Power_Off";
const char DF2301QUsermod::_cmdBrightnessUp[]   PROGMEM = "cmd_Brightness_Up";
const char DF2301QUsermod::_cmdBrightnessDown[] PROGMEM = "cmd_Brightness_Down";
const char DF2301QUsermod::_cmdNextPreset[]     PROGMEM = "cmd_Next_Preset";
const char DF2301QUsermod::_cmdPrevPreset[]     PROGMEM = "cmd_Previous_Preset";
const char DF2301QUsermod::_cmdNextEffect[]     PROGMEM = "cmd_Next_Effect";
const char DF2301QUsermod::_cmdPrevEffect[]     PROGMEM = "cmd_Previous_Effect";
const char DF2301QUsermod::_enableNumberedPresets[] PROGMEM = "Numbered_Presets";
const char DF2301QUsermod::_enableColorCommands[]   PROGMEM = "Color_Commands";
const char DF2301QUsermod::_enableVoiceFeedback[]   PROGMEM = "Voice_Feedback";
const char DF2301QUsermod::_startupSound[]          PROGMEM = "Startup_Sound";
