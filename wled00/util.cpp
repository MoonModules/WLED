#include "wled.h"
#include "fcn_declare.h"
#include "const.h"
#include "util.h"

#ifdef ESP8266
#include "user_interface.h" // for bootloop detection
#include <Hash.h>            // for SHA1 on ESP8266
#else
#include "mbedtls/sha1.h"   // for SHA1 on ESP32
#include "esp_efuse.h"
#include "esp_adc_cal.h"
#include "esp_heap_caps.h"
#endif

//helper to get int value at a position in string
int getNumVal(const String* req, uint16_t pos)
{
  return req->substring(pos+3).toInt();
}


//helper to get int value with in/decrementing support via ~ syntax
void parseNumber(const char* str, byte* val, byte minv, byte maxv)
{
  if (str == nullptr || str[0] == '\0') return;
  if (str[0] == 'r') {*val = random8(minv,maxv?maxv:255); return;} // maxv for random cannot be 0
  bool wrap = false;
  if (str[0] == 'w' && strlen(str) > 1) {str++; wrap = true;}
  if (str[0] == '~') {
    int out = atoi(str +1);
    if (out == 0) {
      if (str[1] == '0') return;
      if (str[1] == '-') {
        *val = (int)(*val -1) < (int)minv ? maxv : min((int)maxv,(*val -1)); //-1, wrap around
      } else {
        *val = (int)(*val +1) > (int)maxv ? minv : max((int)minv,(*val +1)); //+1, wrap around
      }
    } else {
      if (wrap && *val == maxv && out > 0) out = minv;
      else if (wrap && *val == minv && out < 0) out = maxv;
      else {
        out += *val;
        if (out > maxv) out = maxv;
        if (out < minv) out = minv;
      }
      *val = out;
    }
    return;
  } else if (minv == maxv && minv == 0) { // limits "unset" i.e. both 0
    byte p1 = atoi(str);
    const char* str2 = strchr(str,'~'); // min/max range (for preset cycle, e.g. "1~5~")
    if (str2) {
      byte p2 = atoi(++str2);           // skip ~
      if (p2 > 0) {
        while (isdigit(*(++str2)));     // skip digits
        parseNumber(str2, val, p1, p2);
        return;
      }
    }
  }
  *val = atoi(str);
}


bool getVal(JsonVariant elem, byte* val, byte vmin, byte vmax) {
  if (elem.is<int>()) {
		if (elem < 0) return false; //ignore e.g. {"ps":-1}
    *val = elem;
    return true;
  } else if (elem.is<const char*>()) {
    const char* str = elem;
    size_t len = strnlen(str, 12);
    if (len == 0 || len > 10) return false;
    parseNumber(str, val, vmin, vmax);
    return true;
  }
  return false; //key does not exist
}


bool updateVal(const char* req, const char* key, byte* val, byte minv, byte maxv)
{
  const char *v = strstr(req, key);
  if (v) v += strlen(key);
  else return false;
  parseNumber(v, val, minv, maxv);
  return true;
}


//append a numeric setting to string buffer
void sappend(char stype, const char* key, int val)
{
  char ds[] = "d.Sf.";

  switch(stype)
  {
    case 'c': //checkbox
      oappend(ds);
      oappend(key);
      oappend(".checked=");
      oappendi(val);
      oappend(";");
      break;
    case 'v': //numeric
      oappend(ds);
      oappend(key);
      oappend(".value=");
      oappendi(val);
      oappend(";");
      break;
    case 'i': //selectedIndex
      oappend(ds);
      oappend(key);
      oappend(SET_F(".selectedIndex="));
      oappendi(val);
      oappend(";");
      break;
  }
}


//append a string setting to buffer
void sappends(char stype, const char* key, char* val)
{
  switch(stype)
  {
    case 's': {//string (we can interpret val as char*)
      String buf = val;
      //convert "%" to "%%" to make EspAsyncWebServer happy
      //buf.replace("%","%%");
      oappend("d.Sf.");
      oappend(key);
      oappend(".value=\"");
      oappend(buf.c_str());
      oappend("\";");
      break;}
    case 'm': //message
      oappend(SET_F("d.getElementsByClassName"));
      oappend(key);
      oappend(SET_F(".innerHTML=\""));
      oappend(val);
      oappend("\";");
      break;
  }
}


bool oappendi(int i)
{
  char s[16];               // WLEDMM max 32bit integer needs 11 chars (sign + 10) not 10
  snprintf(s, 15, "%d", i); // WLEDMM
  return oappend(s);
}

static bool squeezeStrings = false;
void oappendUseDeflate(bool OnOff) { squeezeStrings = OnOff; }

bool oappend(const char* txt)
{
  String str = squeezeStrings ? String(txt) : String("");
  if (squeezeStrings) {
    // simple fixed-dictionary deflate
    str.replace(F("addField("),    F("adF("));
    str.replace(F("addDropdown("), F("adD("));
    str.replace(F("addOption("),   F("adO("));
    str.replace(F("addInfo("),     F("adI("));
  }
  const char* finalTxt = squeezeStrings ? str.c_str() : txt;

  size_t len = strlen(finalTxt);
  if ((obuf == nullptr) || (olen + len >= SETTINGS_STACK_BUF_SIZE)) { // sanity checks
	  if (obuf == nullptr) { USER_PRINTLN(F("oappend() error: obuf == nullptr."));
	  } else {
	    USER_PRINT(F("oappend() error: buffer full. Increase SETTINGS_STACK_BUF_SIZE for "));
      USER_PRINTF("%2u bytes \t\"", len /*1 + olen + len - SETTINGS_STACK_BUF_SIZE*/);
      USER_PRINT(finalTxt);
      USER_PRINTLN(F("\""));
      errorFlag = ERR_LOW_AJAX_MEM;
	  }
    return false;        // buffer full
  }
  strcpy(obuf + olen, finalTxt);
  olen += len;
  return true;
}


void prepareHostname(char* hostname)
{
  sprintf_P(hostname, "wled-%*s", 6, escapedMac.c_str() + 6);
  const char *pC = serverDescription;
  uint8_t pos = 5;          // keep "wled-"
  while (*pC && pos < 24) { // while !null and not over length
    if (isalnum(*pC)) {     // if the current char is alpha-numeric append it to the hostname
      hostname[pos] = *pC;
      pos++;
    } else if (*pC == ' ' || *pC == '_' || *pC == '-' || *pC == '+' || *pC == '!' || *pC == '?' || *pC == '*') {
      hostname[pos] = '-';
      pos++;
    }
    // else do nothing - no leading hyphens and do not include hyphens for all other characters.
    pC++;
  }
  //last character must not be hyphen
  if (pos > 5) {
    while (pos > 4 && hostname[pos -1] == '-') pos--;
    hostname[pos] = '\0'; // terminate string (leave at least "wled")
  }
}


bool isAsterisksOnly(const char* str, byte maxLen)
{
  for (byte i = 0; i < maxLen; i++) {
    if (str[i] == 0) break;
    if (str[i] != '*') return false;
  }
  //at this point the password contains asterisks only
  return (str[0] != 0); //false on empty string
}


//threading/network callback details: https://github.com/Aircoookie/WLED/pull/2336#discussion_r762276994
bool requestJSONBufferLock(uint8_t module, unsigned timeoutMS)
{
  bool haveLock = false;
  #ifdef ARDUINO_ARCH_ESP32
    // We use a recursive mutex to prevent parallel JSON writes from parallel tasks.
    // This also fixes hanging up for the full timeout interval in cases when the contention is from the same task.
    // see https://github.com/wled/WLED/pull/4089 for more details.
    if (esp32SemTake(jsonBufferLockMutex, timeoutMS) == pdTRUE) haveLock = true;  // WLEDMM must wait longer than suspendStripService timeout = 1500ms
  #else
    // 8266: only wait in case that can_yield() tells us we can yield and delay
    if (can_yield()) {
      unsigned long now = millis();
      while (jsonBufferLock && millis()-now < timeoutMS) delay(1); // wait for fraction for buffer lock // WLEDMM must wait longer than suspendStripService timeout = 1500ms
      if (!jsonBufferLock) haveLock = true;
    }
  #endif

  if (jsonBufferLock || !haveLock) {
    #ifdef ARDUINO_ARCH_ESP32
    if (haveLock) esp32SemGive(jsonBufferLockMutex);  // we got the mutex, but jsonBufferLock says the opposite -> give up
    #endif
    USER_PRINT(F("ERROR: Locking JSON buffer failed! (still locked by "));
    USER_PRINT(jsonBufferLock);
    USER_PRINTLN(")");
    return false; // waiting time-outed
  }

  // success - we keep holding the mutex until releaseJSONBufferLock()
  jsonBufferLock = module ? module : 255;
  DEBUG_PRINT(F("JSON buffer locked. ("));
  DEBUG_PRINT(jsonBufferLock);
  DEBUG_PRINTLN(")");
  fileDoc = &doc;  // used for applying presets (presets.cpp)
  doc.clear();
  return true;
}


void releaseJSONBufferLock()
{
  DEBUG_PRINT(F("JSON buffer released. ("));
  DEBUG_PRINT(jsonBufferLock);
  DEBUG_PRINTLN(")");
  fileDoc = nullptr;
  jsonBufferLock = 0;
  #ifdef ARDUINO_ARCH_ESP32
  esp32SemGive(jsonBufferLockMutex); // return the mutex
  #endif
}


// extracts effect mode (or palette) name from names serialized string
// caller must provide large enough buffer for name (including SR extensions)! maxLen is (buffersize - 1)
uint8_t extractModeName(uint8_t mode, const char *src, char *dest, uint8_t maxLen)
{
  if (src == JSON_mode_names || src == nullptr) {
    if (mode < strip.getModeCount()) {
      char lineBuffer[256] = { '\0' };
      //strcpy_P(lineBuffer, (const char*)pgm_read_dword(&(WS2812FX::_modeData[mode])));
      strncpy_P(lineBuffer, strip.getModeData(mode), sizeof(lineBuffer)/sizeof(char)-1);
      lineBuffer[sizeof(lineBuffer)/sizeof(char)-1] = '\0'; // terminate string
      size_t len = strlen(lineBuffer);
      size_t j = 0;
      for (; j < maxLen && j < len; j++) {
        if (lineBuffer[j] == '\0' || lineBuffer[j] == '@') break;
        dest[j] = lineBuffer[j];
      }
      dest[j] = 0; // terminate string
      return strlen(dest);
    } else return 0;
  }

  if (src == JSON_palette_names && mode > (GRADIENT_PALETTE_COUNT + 13)) {
    snprintf_P(dest, maxLen, PSTR("~ Custom %d ~"), 255-mode);
    dest[maxLen] = '\0';
    return strlen(dest);
  }

  uint8_t qComma = 0;
  bool insideQuotes = false;
  uint8_t printedChars = 0;
  char singleJsonSymbol;
  size_t len = strlen_P(src);

  // Find the mode name in JSON
  for (size_t i = 0; i < len; i++) {
    singleJsonSymbol = pgm_read_byte_near(src + i);
    if (singleJsonSymbol == '\0') break;
    if (singleJsonSymbol == '@' && insideQuotes && qComma == mode) break; //stop when SR extension encountered
    switch (singleJsonSymbol) {
      case '"':
        insideQuotes = !insideQuotes;
        break;
      case '[': // falls through
      case ']':
        break;
      case ',':
        if (!insideQuotes) qComma++;
         // falls through
      default:
        if (!insideQuotes || (qComma != mode)) break;
        dest[printedChars++] = singleJsonSymbol;
    }
    if ((qComma > mode) || (printedChars >= maxLen)) break;
  }
  dest[printedChars] = '\0';
  return strlen(dest);
}


// extracts effect slider data (1st group after @) -> maxLen is (buffersize - 1)
uint8_t extractModeSlider(uint8_t mode, uint8_t slider, char *dest, uint8_t maxLen, uint8_t *var)
{
  dest[0] = '\0'; // start by clearing buffer

  if (mode < strip.getModeCount()) {
    String lineBuffer = FPSTR(strip.getModeData(mode));
    if (lineBuffer.length() > 0) {
      int16_t start = lineBuffer.indexOf('@');
      int16_t stop  = lineBuffer.indexOf(';', start);
      if (start>0 && stop>0) {
        String names = lineBuffer.substring(start, stop); // include @
        int16_t nameBegin = 1, nameEnd, nameDefault;
        if (slider < 10) {
          for (size_t i=0; i<=slider; i++) {
            const char *tmpstr;
            dest[0] = '\0'; //clear dest buffer
            if (nameBegin == 0) break; // there are no more names
            nameEnd = names.indexOf(',', nameBegin);
            if (i == slider) {
              nameDefault = names.indexOf('=', nameBegin); // find default value
              if (nameDefault > 0 && var && ((nameEnd>0 && nameDefault<nameEnd) || nameEnd<0)) {
                *var = (uint8_t)atoi(names.substring(nameDefault+1).c_str());
              }
              if (names.charAt(nameBegin) == '!') {
                switch (slider) {
                  case  0: tmpstr = PSTR("FX Speed");     break;
                  case  1: tmpstr = PSTR("FX Intensity"); break;
                  case  2: tmpstr = PSTR("FX Custom 1");  break;
                  case  3: tmpstr = PSTR("FX Custom 2");  break;
                  case  4: tmpstr = PSTR("FX Custom 3");  break;
                  default: tmpstr = PSTR("FX Custom");    break;
                }
                strncpy_P(dest, tmpstr, maxLen); // copy the name into buffer (replacing previous)
                dest[maxLen-1] = '\0';
              } else {
                // WLEDMM bugfix for WLED-MM #272
                // names.substring(...).c_str() returns a pointer to a temporary; it’s invalid by the next statement. Added result buffer "sub" to avoid use-after-free
                String sub = (nameEnd<0) ? names.substring(nameBegin) : names.substring(nameBegin, nameEnd); // special handling in case we did not find "," (last name)
                strlcpy(dest, sub.c_str(), maxLen); // copy the name into buffer (replacing previous)
			  }
            }
            nameBegin = nameEnd+1; // next name (if "," is not found it will be 0)
          } // next slider
        } else if (slider == 255) {
          // palette
          strlcpy(dest, "pal", maxLen);
          names = lineBuffer.substring(stop+1); // stop has index of color slot names
          nameBegin = names.indexOf(';'); // look for palette
          if (nameBegin >= 0) {
            nameEnd = names.indexOf(';', nameBegin+1);
            if (!isdigit(names[nameBegin+1])) nameBegin = names.indexOf('=', nameBegin+1); // look for default value
            if (nameEnd >= 0 && nameBegin > nameEnd) nameBegin = -1;
            if (nameBegin >= 0 && var) {
              *var = (uint8_t)atoi(names.substring(nameBegin+1).c_str());
            }
          }
        }
        // we have slider name (including default value) in the dest buffer
        for (size_t i=0; i<strlen(dest); i++) if (dest[i]=='=') { dest[i]='\0'; break; } // truncate default value

      } else {
        // defaults to just speed and intensity since there is no slider data
        switch (slider) {
          case 0:  strncpy_P(dest, PSTR("FX Speed"), maxLen); break;
          case 1:  strncpy_P(dest, PSTR("FX Intensity"), maxLen); break;
        }
        dest[maxLen] = '\0'; // strncpy does not necessarily null terminate string
      }
    }
    return strlen(dest);
  }
  return 0;
}


// extracts mode parameter defaults from last section of mode data (e.g. "Juggle@!,Trail;!,!,;!;sx=16,ix=240,1d")
int16_t extractModeDefaults(uint8_t mode, const char *segVar)
{
  if (mode < strip.getModeCount()) {
    char lineBuffer[256] = { '\0' };
    strncpy_P(lineBuffer, strip.getModeData(mode), sizeof(lineBuffer)/sizeof(char)-1);
    lineBuffer[sizeof(lineBuffer)/sizeof(char)-1] = '\0'; // terminate string
    if (lineBuffer[0] != 0) {
      char* startPtr = strrchr(lineBuffer, ';'); // last ";" in FX data
      if (!startPtr) return -1;

      char* stopPtr = strstr(startPtr, segVar);
      if (!stopPtr) return -1;

      stopPtr += strlen(segVar) +1; // skip "="
      return atoi(stopPtr);
    }
  }
  return -1;
}


void checkSettingsPIN(const char* pin) {
  if (!pin) return;
  if (!correctPIN && millis() - lastEditTime < PIN_RETRY_COOLDOWN) return; // guard against PIN brute force
  bool correctBefore = correctPIN;
  correctPIN = (strlen(settingsPIN) == 0 || strncmp(settingsPIN, pin, 4) == 0);
  if (correctBefore != correctPIN) createEditHandler(correctPIN);
  lastEditTime = millis();
}


uint16_t crc16(const unsigned char* data_p, size_t length) {
  uint8_t x;
  uint16_t crc = 0xFFFF;
  if (!length) return 0x1D0F;
  while (length--) {
    x = crc >> 8 ^ *data_p++;
    x ^= x>>4;
    crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
  }
  return crc;
}

// fastled beatsin: 1:1 replacements to remove the use of fastled sin16()
// Generates a 16-bit sine wave at a given BPM that oscillates within a given range. see fastled for details.
uint16_t beatsin88_t(accum88 beats_per_minute_88, uint16_t lowest, uint16_t highest, uint32_t timebase, uint16_t phase_offset)
{
    uint16_t beat = beat88( beats_per_minute_88, timebase);
    uint16_t beatsin (sin16_t( beat + phase_offset) + 32768);
    uint16_t rangewidth = highest - lowest;
    uint16_t scaledbeat = scale16( beatsin, rangewidth);
    uint16_t result = lowest + scaledbeat;
    return result;
}

// Generates a 16-bit sine wave at a given BPM that oscillates within a given range. see fastled for details.
uint16_t beatsin16_t(accum88 beats_per_minute, uint16_t lowest, uint16_t highest, uint32_t timebase, uint16_t phase_offset)
{
    uint16_t beat = beat16( beats_per_minute, timebase);
    uint16_t beatsin = (sin16_t( beat + phase_offset) + 32768);
    uint16_t rangewidth = highest - lowest;
    uint16_t scaledbeat = scale16( beatsin, rangewidth);
    uint16_t result = lowest + scaledbeat;
    return result;
}

// Generates an 8-bit sine wave at a given BPM that oscillates within a given range. see fastled for details.
uint8_t beatsin8_t(accum88 beats_per_minute, uint8_t lowest, uint8_t highest, uint32_t timebase, uint8_t phase_offset)
{
    uint8_t beat = beat8( beats_per_minute, timebase);
    uint8_t beatsin = sin8_t( beat + phase_offset);
    uint8_t rangewidth = highest - lowest;
    uint8_t scaledbeat = scale8( beatsin, rangewidth);
    uint8_t result = lowest + scaledbeat;
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Begin simulateSound (to enable audio enhanced effects to display something)
///////////////////////////////////////////////////////////////////////////////
// Currently 4 types defined, to be fine tuned and new types added
// (only 2 used as stored in 1 bit in segment options, consider switching to a single global simulation type)
typedef enum UM_SoundSimulations {
  UMS_BeatSin = 0,
  UMS_WeWillRockYou,
  UMS_10_13,
  UMS_14_3
} um_soundSimulations_t;

um_data_t* simulateSound(uint8_t simulationId)
{
  static uint8_t samplePeak;
  static float   FFT_MajorPeak;
  static uint8_t maxVol;
  static uint8_t binNum;

  static float    volumeSmth;
  static uint16_t volumeRaw;
  static float    my_magnitude;
  static uint16_t zeroCrossingCount = 0; // number of zero crossings in the current batch of 512 samples

  //arrays
  uint8_t *fftResult;

  static um_data_t* um_data = nullptr;

  if (!um_data) {
    //claim storage for arrays
    fftResult = (uint8_t *)malloc(sizeof(uint8_t) * 16);
    //fftResult = (uint8_t *)d_malloc(sizeof(uint8_t) * 16); // might potentially fail with nullptr. We don't have a solution or fallback for this case.

    // initialize um_data pointer structure
    // NOTE!!!
    // This may change as AudioReactive usermod may change
    um_data = new um_data_t;
    um_data->u_size = 12;
    um_data->u_type = new um_types_t[um_data->u_size];
    um_data->u_data = new void*[um_data->u_size];
    um_data->u_data[0] = &volumeSmth;
    um_data->u_data[1] = &volumeRaw;
    um_data->u_data[2] = fftResult;
    um_data->u_data[3] = &samplePeak;
    um_data->u_data[4] = &FFT_MajorPeak;
    um_data->u_data[5] = &my_magnitude;
    um_data->u_data[6] = &maxVol;
    um_data->u_data[7] = &binNum;
    um_data->u_data[8]  = &FFT_MajorPeak; // dummy (FFT Peak smoothed)
    um_data->u_data[9]  = &volumeSmth;    // dummy (soundPressure)
    um_data->u_data[10] = &volumeSmth;    // dummy (agcSensitivity)
    um_data->u_data[11] = &zeroCrossingCount;
  } else {
    // get arrays from um_data
    fftResult =  (uint8_t*)um_data->u_data[2];
  }

  uint32_t ms = millis();

  switch (simulationId) {
    default:
    case UMS_BeatSin:
      for (int i = 0; i<16; i++)
        fftResult[i] = beatsin8_t(120 / (i+1), 0, 255);
        // fftResult[i] = (beatsin8_t(120, 0, 255) + (256/16 * i)) % 256;
        volumeSmth = fftResult[8];
      break;
    case UMS_WeWillRockYou:
      if (ms%2000 < 200) {
        volumeSmth = random8(255);
        for (int i = 0; i<5; i++)
          fftResult[i] = random8(255);
      }
      else if (ms%2000 < 400) {
        volumeSmth = 0;
        for (int i = 0; i<16; i++)
          fftResult[i] = 0;
      }
      else if (ms%2000 < 600) {
        volumeSmth = random8(255);
        for (int i = 5; i<11; i++)
          fftResult[i] = random8(255);
      }
      else if (ms%2000 < 800) {
        volumeSmth = 0;
        for (int i = 0; i<16; i++)
          fftResult[i] = 0;
      }
      else if (ms%2000 < 1000) {
        volumeSmth = random8(255);
        for (int i = 11; i<16; i++)
          fftResult[i] = random8(255);
      }
      else {
        volumeSmth = 0;
        for (int i = 0; i<16; i++)
          fftResult[i] = 0;
      }
      break;
    case UMS_10_13:
      for (int i = 0; i<16; i++)
        fftResult[i] = perlin8(beatsin8_t(90 / (i+1), 0, 200)*15 + (ms>>10), ms>>3);
        volumeSmth = fftResult[8];
      break;
    case UMS_14_3:
      for (int i = 0; i<16; i++)
        fftResult[i] = perlin8(beatsin8_t(120 / (i+1), 10, 30)*10 + (ms>>14), ms>>3);
      volumeSmth = fftResult[8];
      break;
  }

  samplePeak    = random8() > 250;
  FFT_MajorPeak = 21 + (volumeSmth*volumeSmth) / 8.0f; // walk thru full range of 21hz...8200hz
  maxVol        = 31;  // this gets feedback fro UI
  binNum        = 8;   // this gets feedback fro UI
  volumeRaw = volumeSmth;
  my_magnitude = 10000.0f / 8.0f; //no idea if 10000 is a good value for FFT_Magnitude ???
  if (volumeSmth < 1 ) my_magnitude = 0.001f;             // noise gate closed - mute
  zeroCrossingCount = floorf(FFT_MajorPeak / 36.0f); // 9Khz max frequency => 255 zero crossings

  return um_data;
}

//WLEDMM enumerateLedmaps moved to FX_fcn.cpp

//WLEDMM netmindz ar palette
CRGB getCRGBForBand(int x, uint8_t *fftResult, int pal) { 
  CRGB value;
  CHSV hsv;
  if(pal == 71) { // bit hacky to use palette id here, but don't want to litter the code with lots of different methods. TODO: add enum for palette creation type
    if(x == 1) {
      value = CRGB(fftResult[10]/2, fftResult[4]/2, fftResult[0]/2);
    }
    else if(x == 255) {
      value = CRGB(fftResult[10]/2, fftResult[0]/2, fftResult[4]/2);
    } 
    else {
      value = CRGB(fftResult[0]/2, fftResult[4]/2, fftResult[10]/2);
    } 
  }
  else if(pal == 72) {
    int b = map(x, 1, 255, 0, 10); // convert palette position to lower half of freq band
    hsv = CHSV(fftResult[b], 255, map(fftResult[b], 0, 255, 30, 255));  // pick hue
    hsv2rgb_rainbow(hsv, value);  // convert to R,G,B
  }
  else if(pal == 73) {
    int b = map(x, 0, 255, 0, 8); // convert palette position to lower half of freq band
    hsv = CHSV(uint8_t(fftResult[b]), 255, x);
    hsv2rgb_rainbow(hsv, value);  // convert to R,G,B
  }

  return value;
}

/*
 * Returns a new, random color wheel index with a minimum distance of 42 from pos.
 */
uint8_t get_random_wheel_index(uint8_t pos) {
  uint8_t r = 0, x = 0, y = 0, d = 0;
  while (d < 42) {
    r = hw_random8();
    x = abs(pos - r);
    y = 255 - x;
    d = MIN(x, y);
  }
  return r;
}

// float version of map() - WLEDMM not used
//float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
//  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
//}

//uint32_t hashInt(uint32_t s) { // WLEDMM not used
//  // borrowed from https://stackoverflow.com/questions/664014/what-integer-hash-function-are-good-that-accepts-an-integer-hash-key
//  s = ((s >> 16) ^ s) * 0x45d9f3b;
//  s = ((s >> 16) ^ s) * 0x45d9f3b;
//  return (s >> 16) ^ s;
//}


// WLEDMM extended "trim string" function to support enumerateLedmaps
// The function takes char* as input, and removes all leading and trailing "decorations" like spaces, tabs, line endings, quotes, colons
// The conversion is "in place" (destructive).
// example: cleanUpName("\t \"Ring241x 60/9 squeeze \" ,\r") returns "Ring241x 60/9 squeeze"
// 
// Null pointer and zero size "C strings" are handled correctly.
// Will not work with flash strings. Unicode encoded multi-byte char strings may get corrupted.
//
static const char *unwantedChars = "\r\n\t\b ,;:\"\'`´\\"; // list of chars to delete
//
char *cleanUpName(char *in) {
  if (nullptr == in) return(in);
  size_t len = strlen(in);
  if (len == 0) return(in);

  // delete trailing garbage
  while ((len > 0) && (strchr(unwantedChars, in[len-1]) != nullptr)) {
    in[len-1] = '\0'; // deletes last char
    len--;
  }
  // delete leading garbage
  while ((len > 0) && (strchr(unwantedChars, in[0]) != nullptr)) {
    (void) memmove(in, in+1, len); // shifts string left by one
    len--;
  }
  
  return(in);
}


// 32 bit hardware random number generator, inlining uses more code, use hw_random16() if speed is critical (see fcn_declare.h)
uint32_t hw_random(uint32_t upperlimit) {
  uint32_t rnd = hw_random();
  uint64_t scaled = uint64_t(rnd) * uint64_t(upperlimit);
  return scaled >> 32;
}

int32_t hw_random(int32_t lowerlimit, int32_t upperlimit) {
  if(lowerlimit >= upperlimit) {
    return lowerlimit;
  }
  uint32_t diff = upperlimit - lowerlimit;
  return hw_random(diff) + lowerlimit;
}


// memory allocation functions with minimum free heap size check
#ifdef ESP8266
static void *validateFreeHeap(void *buffer) {
  // make sure there is enough free heap left if buffer was allocated in DRAM region, free it if not
  // note: ESP826 needs very little contiguous heap for webserver, checking total free heap works better
  if (getFreeHeapSize() < MIN_HEAP_SIZE) {
    free(buffer);
    return nullptr;
  }
  return buffer;
}

void *d_malloc(size_t size) {
  // note: using "if (getContiguousFreeHeap() > MIN_HEAP_SIZE + size)" did perform worse in tests with regards to keeping heap healthy and UI working
  void *buffer = malloc(size);
  return validateFreeHeap(buffer);
}

void *d_malloc_only(size_t size) {
  return d_malloc(size);
}

void *d_calloc(size_t count, size_t size) {
  void *buffer = calloc(count, size);
  return validateFreeHeap(buffer);
}

// realloc with malloc fallback, note: on ESPS8266 there is no safe way to ensure MIN_HEAP_SIZE during realloc()s, free buffer and allocate new one
void *d_realloc_malloc(void *ptr, size_t size) {
  //void *buffer = realloc(ptr, size);
  //buffer = validateFreeHeap(buffer);
  //if (buffer) return buffer; // realloc successful
  //d_free(ptr); // free old buffer if realloc failed (or min heap was exceeded)
  //return d_malloc(size); // fallback to malloc
  free(ptr);
  return d_malloc(size);
}

void *d_realloc_malloc_nofree(void *ptr, size_t size) {
  void *buffer = realloc(ptr, size);
  //buffer = validateFreeHeap(buffer); violates contract
  return buffer; // realloc done
}


void d_free(void *ptr) { free(ptr); }

void *p_malloc(size_t size) { return d_malloc(size); }
void *p_calloc(size_t count, size_t size) { return d_calloc(count, size); }
void *p_realloc_malloc(void *ptr, size_t size) { return d_realloc_malloc(ptr, size); }
void *p_realloc_malloc_nofree(void *ptr, size_t size) { return d_realloc_malloc_nofree(ptr, size); }
void p_free(void *ptr) { free(ptr); }

#else

static size_t lastHeap = 65535;
static size_t lastMinHeap = 65535;
WLED_create_spinlock(heapStatusMux); // to prevent race conditions on lastHeap and lastMinHeap

inline static void d_measureHeap(void) {
#ifdef WLEDMM_FILEWAIT  // only when we don't use the RMTHI driver
  if (!strip.isUpdating())  // skip measurement while sending out LEDs - prevents flickering
#endif
  {
    size_t newlastHeap    = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    size_t newlastMinHeap = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    portENTER_CRITICAL(&heapStatusMux); // atomic operation
      lastHeap = newlastHeap;
      lastMinHeap = newlastMinHeap;
    portEXIT_CRITICAL(&heapStatusMux);
  }
}

size_t d_measureContiguousFreeHeap(void) { 
  d_measureHeap();
  return lastMinHeap;
} // returns largest contiguous free block // WLEDMM may glitch, too

size_t d_measureFreeHeap(void) {
  d_measureHeap();
  return lastHeap;
} // returns free heap (ESP.getFreeHeap() can include other memory types) // WLEDMM can cause LED glitches

// early check to avoid heap fragmentation: when PSRAM is available, we reject DRAM request if remaining heap possibly gets too low.
//   This check is not exact - in case of strong heap fragmentation, there might be multiple chunks of similar sizes.
//   However it still improves stability in low-heap situations (tested).
static inline bool isOkForDRAMHeap(size_t amount) {
#if defined(BOARD_HAS_PSRAM) || (ESP_IDF_VERSION_MAJOR > 3)
  // if (!psramFound()) return true; // No PSRAM -> accept everything // disabled - increases fragmentation
  size_t avail = d_measureContiguousFreeHeap();
  if ((amount < avail) && (avail - amount > MIN_HEAP_SIZE)) return true;
  else {
    DEBUG_PRINTF("* isOkForDRAMHeap() DRAM allocation rejected (%u bytes requested, %u-%u available).\n", amount, avail, MIN_HEAP_SIZE);
    return(false);
  }
  #else
  return true; // No PSRAM -> no other options
  #endif
}

static void *validateFreeHeap(void *buffer) {
  // make sure there is enough free heap left if buffer was allocated in DRAM region, free it if not
  // TODO: between allocate and free, heap can run low (async web access), only IDF V5 allows for a pre-allocation-check of all free blocks
  if (buffer == nullptr) return buffer; // early exit, nothing to check
  if ((uintptr_t)buffer > SOC_DRAM_LOW && (uintptr_t)buffer < SOC_DRAM_HIGH) { 
    size_t avail = d_measureContiguousFreeHeap();
    if (avail < MIN_HEAP_SIZE) { 
      heap_caps_free(buffer);
      USER_PRINTF("* validateFreeHeap() DRAM allocation rejected - largest remaining chunk too small (%u bytes).\n", avail);
      d_measureHeap(); // update statistics after free
      return nullptr;
    }
  }
  return buffer;
}

#ifdef BOARD_HAS_PSRAM
#define RTC_RAM_THRESHOLD 1024 // use RTC RAM for allocations smaller than this size
#else
#define RTC_RAM_THRESHOLD (psramFound() ? 1024 : 65535) // without PSRAM, allow any size into RTC RAM (useful especially on S2 without PSRAM)
#endif

void *d_malloc(size_t size) {
  void *buffer = nullptr;
  #if !defined(CONFIG_IDF_TARGET_ESP32)
  // the newer ESP32 variants have byte-accessible fast RTC memory that can be used as heap, access speed is on-par with DRAM
  // the system does prefer normal DRAM until full, since free RTC memory is ~7.5k only, its below the minimum heap threshold and needs to be allocated explicitly
  // use RTC RAM for small allocations or if DRAM is running low to improve fragmentation
  if (size <= RTC_RAM_THRESHOLD || getContiguousFreeHeap() < 2*MIN_HEAP_SIZE + size) {
    //buffer = heap_caps_malloc_prefer(size, 2, MALLOC_CAP_RTCRAM, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    buffer = heap_caps_malloc(size, MALLOC_CAP_RTCRAM | MALLOC_CAP_8BIT);
    DEBUG_PRINTF("* d_malloc() trying RTCRAM (%u bytes) - %s.\n", size, buffer?"success":"fail");
  }
  #endif

  if ((buffer == nullptr) && isOkForDRAMHeap(size)) // no RTC RAM allocation: use DRAM
    buffer = heap_caps_malloc(size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT); // allocate in any available heap memory
  buffer = validateFreeHeap(buffer); // make sure there is enough free heap left

  #if defined(BOARD_HAS_PSRAM) || (ESP_IDF_VERSION_MAJOR > 3) // WLEDMM always try PSRAM (auto-detected)
  if (!buffer && psramFound()) {
    DEBUG_PRINTF("* d_malloc() using PSRAM(%u bytes) fsllback.\n", size);
    return heap_caps_malloc_prefer(size, 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT, MALLOC_CAP_DEFAULT); // DRAM failed,try PSRAM if available
  }
  #endif

  if (!buffer) { errorFlag = ERR_LOW_MEM; USER_PRINTF("* d_malloc() failed (%u bytes) !\n", size); }
  else if (errorFlag == ERR_LOW_MEM) errorFlag = ERR_NONE; // reset mem error flag
  return buffer;
}

void *d_malloc_only(size_t size) {
  // variant of d_malloc that only allocates from "internal" DRAM (no PSRAM fallback) - MIN_HEAP_SIZE checking relaxed to post-malloc only
  void *buffer = nullptr;  
  #if !defined(CONFIG_IDF_TARGET_ESP32) // try RTCRAM on newer chips
  if (size <= RTC_RAM_THRESHOLD || getContiguousFreeHeap() < 2*MIN_HEAP_SIZE + size) {
    buffer = heap_caps_malloc(size, MALLOC_CAP_RTCRAM | MALLOC_CAP_8BIT);
    DEBUG_PRINTF("* d_malloc() trying RTCRAM (%u bytes) - %s.\n", size, buffer?"success":"fail");
  }
  #endif
  if (!buffer) buffer = heap_caps_malloc(size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  buffer = validateFreeHeap(buffer); // make sure there is enough free heap left

  if (!buffer) { errorFlag = ERR_LOW_MEM; USER_PRINTF("* d_malloc_only() failed (%u bytes) !\n", size); }
  else if (errorFlag == ERR_LOW_MEM) errorFlag = ERR_NONE; // reset mem error flag
  return buffer;
}

void *d_calloc(size_t count, size_t size) {
  // similar to d_malloc but uses heap_caps_calloc
  void *buffer = nullptr;
  #if !defined(CONFIG_IDF_TARGET_ESP32)
  if ((size * count) <= RTC_RAM_THRESHOLD || getContiguousFreeHeap() < 2*MIN_HEAP_SIZE + (size * count)) {
    //buffer = heap_caps_calloc_prefer(count, size, 2, MALLOC_CAP_RTCRAM, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    buffer = heap_caps_calloc(count, size, MALLOC_CAP_RTCRAM | MALLOC_CAP_8BIT);
    DEBUG_PRINTF("* d_calloc() trying RTCRAM (%u bytes) - %s.\n", size*count, buffer?"success":"fail");
  }
  #endif

  if ((buffer == nullptr) && isOkForDRAMHeap(size*count)) // no RTC RAM allocation: use DRAM
    buffer = heap_caps_calloc(count, size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT); // allocate in any available heap memory
  buffer = validateFreeHeap(buffer); // make sure there is enough free heap left

  #if defined(BOARD_HAS_PSRAM) || (ESP_IDF_VERSION_MAJOR > 3) // WLEDMM always try PSRAM (auto-detected)
  if (!buffer && psramFound()) {
    DEBUG_PRINTF("* d_calloc() using PSRAM (%u bytes) fallback.\n", size*count);
    return heap_caps_calloc_prefer(count, size, 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT, MALLOC_CAP_DEFAULT); // DRAM failed,try PSRAM if available
  }
  #endif
  if (!buffer) { errorFlag = ERR_LOW_MEM; USER_PRINTF("* d_calloc() failed (%u bytes) !\n", size*count); }
  else if (errorFlag == ERR_LOW_MEM) errorFlag = ERR_NONE; // reset mem error flag
  return buffer;
}

// realloc with malloc fallback, original buffer is freed if realloc fails but not copied!
void *d_realloc_malloc(void *ptr, size_t size) {
  #if defined(BOARD_HAS_PSRAM) || (ESP_IDF_VERSION_MAJOR > 3) // WLEDMM always try PSRAM (auto-detected)
    void *buffer = heap_caps_realloc_prefer(ptr, size, 3, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT, MALLOC_CAP_DEFAULT);
  #else
    void *buffer = heap_caps_realloc(ptr, size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  #endif
  void *bufferNew = buffer;
  buffer = validateFreeHeap(buffer);
  if (buffer) return buffer;   // realloc successful
  if (!bufferNew) d_free(ptr); // free old buffer if realloc failed; don't double-free an invalid pointer if min heap was exceeded
  DEBUG_PRINTF("* d_realloc_malloc(): realloc failed (%u bytes), trying malloc.\n", size);
  return d_malloc(size);       // fallback to malloc
}

// realloc without malloc fallback, original buffer not changed if realloc fails
void *d_realloc_malloc_nofree(void *ptr, size_t size) {
  DEBUG_PRINTF("* d_realloc_malloc_nofree() realloc to %u bytes requested.\n", size);
  void* buffer = nullptr;
  #if (ESP_IDF_VERSION_MAJOR > 3)
    // only basic sanity checks possible: prefer PSRAM if DRAM is low
    size_t oldSize = ptr ? heap_caps_get_allocated_size(ptr) : 0; // heap_caps_get_allocated_size crashes on nullptr
    size_t delta = (size > oldSize) ? (size - oldSize) : 0;
    if ((delta == 0) || isOkForDRAMHeap(delta)) {     // prefer DRAM
      buffer = heap_caps_realloc_prefer(ptr, size, 3, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT, MALLOC_CAP_DEFAULT);
    } else {                                          // prefer PSRAM
      buffer = heap_caps_realloc_prefer(ptr, size, 3, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT, MALLOC_CAP_DEFAULT);
    }
  #else
    // V3 lacks heap_caps_get_allocated_size() -> no sanity check
    buffer = heap_caps_realloc_prefer(ptr, size, 2, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT, MALLOC_CAP_DEFAULT);
  #endif
  //buffer = validateFreeHeap(buffer); // violates contract
  if (!buffer) { USER_PRINTF("* d_realloc_malloc_nofree() failed (%u bytes) !\n", size); }
  return buffer;
}

void d_free(void *ptr) { heap_caps_free(ptr); }
void p_free(void *ptr) { heap_caps_free(ptr); }

#if defined(BOARD_HAS_PSRAM) || (ESP_IDF_VERSION_MAJOR > 3)  // V4 can auto-detect PSRAM
// p_xalloc: prefer PSRAM, use DRAM as fallback
void *p_malloc(size_t size) {
  if (!psramFound()) return d_malloc(size);
  void *buffer = heap_caps_malloc_prefer(size, 3, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT, MALLOC_CAP_DEFAULT);
  return validateFreeHeap(buffer);
}

void *p_calloc(size_t count, size_t size) {
  // similar to p_malloc but uses heap_caps_calloc
  if (!psramFound()) return d_calloc(count, size);
  void *buffer = heap_caps_calloc_prefer(count, size, 3, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT, MALLOC_CAP_DEFAULT);
  return validateFreeHeap(buffer);
}

// realloc with malloc fallback, original buffer is freed if realloc fails but not copied!
void *p_realloc_malloc(void *ptr, size_t size) {
  if (!psramFound()) return d_realloc_malloc(ptr, size);
  void *buffer = heap_caps_realloc_prefer(ptr, size, 3, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT, MALLOC_CAP_DEFAULT);
  void *bufferNew = buffer;
  buffer = validateFreeHeap(buffer);
  if (buffer) return buffer;   // realloc successful
  if (!bufferNew) p_free(ptr); // free old buffer if realloc failed; don't double-free an invalid pointer if min heap was exceeded
  return p_malloc(size); // fallback to malloc
}

// realloc without malloc fallback, original buffer not changed if realloc fails
void *p_realloc_malloc_nofree(void *ptr, size_t size) {
  if (!psramFound()) return d_realloc_malloc_nofree(ptr, size);
  void *buffer = heap_caps_realloc_prefer(ptr, size, 3, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT, MALLOC_CAP_DEFAULT);
  //buffer = validateFreeHeap(buffer); // violates contract
  return buffer;
}

#else // NO PSRAM support -> fall back to DRAM
void *p_malloc(size_t size) { return d_malloc(size); }
void *p_calloc(size_t count, size_t size) { return d_calloc(count, size); }
void *p_realloc_malloc(void *ptr, size_t size) { return d_realloc_malloc(ptr, size); }
void *p_realloc_malloc_nofree(void *ptr, size_t size) { return d_realloc_malloc_nofree(ptr, size); }
#endif
#endif


#if 0 // WLEDMM not used yet
// allocation function for buffers like pixel-buffers and segment data
// optimises the use of memory types to balance speed and heap availability, always favours DRAM if possible
// if multiple conflicting types are defined, the lowest bits of "type" take priority (see fcn_declare.h for types)
void *allocate_buffer(size_t size, uint32_t type) {
  void *buffer = nullptr;
  #if CONFIG_IDF_TARGET_ESP32
  // only classic ESP32 has "32bit accessible only" aka IRAM type. Using it frees up normal DRAM for other purposes
  // this memory region is used for IRAM_ATTR functions, whatever is left is unused and can be used for pixel buffers
  // prefer this type over PSRAM as it is slightly faster, except for _pixels where it is on-par as PSRAM-caching does a good job for mostly sequential access
  if (type & BFRALLOC_NOBYTEACCESS) {
    // prefer 32bit region, then PSRAM, fallback to any heap. Note: if adding "INTERNAL"-flag this wont work
    buffer = heap_caps_malloc_prefer(size, 3, MALLOC_CAP_32BIT, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT);
    buffer = validateFreeHeap(buffer);
  }
  else
  #endif
  #if !defined(BOARD_HAS_PSRAM)
  buffer = d_malloc(size);
  #else
  if (type & BFRALLOC_PREFER_DRAM) {
    if (getContiguousFreeHeap() < 3*(MIN_HEAP_SIZE/2) + size && size > PSRAM_THRESHOLD)
      buffer = p_malloc(size); // prefer PSRAM for large allocations & when DRAM is low
    else
      buffer = d_malloc(size); // allocate in DRAM if enough free heap is available, PSRAM as fallback
  }
  else if (type & BFRALLOC_ENFORCE_DRAM)
    buffer = heap_caps_malloc(size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT); // use DRAM only, otherwise return nullptr
  else if (type & BFRALLOC_PREFER_PSRAM) {
    // if DRAM is plenty, prefer it over PSRAM for speed, reserve enough DRAM for segment data: if MAX_SEGMENT_DATA is exceeded, always uses PSRAM
    if (getContiguousFreeHeap() > 4*MIN_HEAP_SIZE + size + ((uint32_t)(MAX_SEGMENT_DATA - Segment::getUsedSegmentData())))
      buffer = d_malloc(size);
    else
      buffer = p_malloc(size); // prefer PSRAM
  }
  else if (type & BFRALLOC_ENFORCE_PSRAM)
    buffer = heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); // use PSRAM only, otherwise return nullptr
  buffer = validateFreeHeap(buffer);
  #endif
  if (buffer && (type & BFRALLOC_CLEAR))
    memset(buffer, 0, size); // clear allocated buffer

  return buffer;
}
#endif


// Platform-agnostic SHA1 computation from String input
String computeSHA1(const String& input) {
  #ifdef ESP8266
    return sha1(input); // ESP8266 has built-in sha1() function
  #else
    // ESP32: Compute SHA1 hash using mbedtls
    unsigned char shaResult[20]; // SHA1 produces 20 bytes
    mbedtls_sha1_context ctx;

    mbedtls_sha1_init(&ctx);
    mbedtls_sha1_starts_ret(&ctx);
    mbedtls_sha1_update_ret(&ctx, (const unsigned char*)input.c_str(), input.length());
    mbedtls_sha1_finish_ret(&ctx, shaResult);
    mbedtls_sha1_free(&ctx);

    // Convert to hexadecimal string
    char hexString[41];
    for (int i = 0; i < 20; i++) {
      sprintf(&hexString[i*2], "%02x", shaResult[i]);
    }
    hexString[40] = '\0';

    return String(hexString);
  #endif
}

#ifdef ESP32
String generateDeviceFingerprint() {
  uint32_t fp[2] = {0, 0}; // create 64 bit fingerprint
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  esp_efuse_mac_get_default((uint8_t*)fp);
  fp[1] ^= ESP.getFlashChipSize();
  #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 4)
  fp[0] ^= chip_info.full_revision | (chip_info.model << 16);
  #else
  fp[0] ^= chip_info.revision | (chip_info.model << 16);
  #endif
  // mix in ADC calibration data:
  esp_adc_cal_characteristics_t ch;
  #if SOC_ADC_MAX_BITWIDTH == 13 // S2 has 13 bit ADC
  constexpr auto myBIT_WIDTH = ADC_WIDTH_BIT_13;
  #else
  constexpr auto myBIT_WIDTH = ADC_WIDTH_BIT_12;
  #endif
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, myBIT_WIDTH, 1100, &ch);
  fp[0] ^= ch.coeff_a;
  fp[1] ^= ch.coeff_b;
  if (ch.low_curve) {
    for (int i = 0; i < 8; i++) {
      fp[0] ^= ch.low_curve[i];
    }
  }
  if (ch.high_curve) {
    for (int i = 0; i < 8; i++) {
      fp[1] ^= ch.high_curve[i];
    }
  }
  char fp_string[17];  // 16 hex chars + null terminator
  sprintf(fp_string, "%08X%08X", fp[1], fp[0]);
  return String(fp_string);
}

#else // ESP8266
String generateDeviceFingerprint() {
  uint32_t fp[2] = {0, 0}; // create 64 bit fingerprint
  WiFi.macAddress((uint8_t*)&fp); // use MAC address as fingerprint base
  fp[0] ^= ESP.getFlashChipId();
  fp[1] ^= ESP.getFlashChipSize() | ESP.getFlashChipVendorId() << 16;
  char fp_string[17];  // 16 hex chars + null terminator
  sprintf(fp_string, "%08X%08X", fp[1], fp[0]);
  return String(fp_string);
}
#endif

// Generate a device ID based on SHA1 hash of MAC address salted with other unique device info
// Returns: original SHA1 + last 2 chars of double-hashed SHA1 (42 chars total)
String getDeviceId() {
  static String cachedDeviceId = "";
  if (cachedDeviceId.length() > 0) return cachedDeviceId;
  // The device string is deterministic as it needs to be consistent for the same device, even after a full flash erase
  // MAC is salted with other consistent device info to avoid rainbow table attacks.
  // If the MAC address is known by malicious actors, they could precompute SHA1 hashes to impersonate devices,
  // but as WLED developers are just looking at statistics and not authenticating devices, this is acceptable.
  // If the usage data was exfiltrated, you could not easily determine the MAC from the device ID without brute forcing SHA1

  String firstHash = computeSHA1(generateDeviceFingerprint());

  // Second hash: SHA1 of the first hash
  String secondHash = computeSHA1(firstHash);

  // Concatenate first hash + last 2 chars of second hash
  cachedDeviceId = firstHash + secondHash.substring(38);

  return cachedDeviceId;
}

/*
 * Fixed point integer based Perlin noise functions by @dedehai
 * Note: optimized for speed and to mimic fastled inoise functions, not for accuracy or best randomness
 */
#define PERLIN_SHIFT 1

// calculate gradient for corner from hash value
static inline __attribute__((always_inline)) int32_t hashToGradient(uint32_t h) {
  // using more steps yields more "detailed" perlin noise but looks less like the original fastled version (adjust PERLIN_SHIFT to compensate, also changes range and needs proper adustment)
  // return (h & 0xFF) - 128; // use PERLIN_SHIFT 7
  // return (h & 0x0F) - 8; // use PERLIN_SHIFT 3
  // return (h & 0x07) - 4; // use PERLIN_SHIFT 2
  return (h & 0x03) - 2; // use PERLIN_SHIFT 1 -> closest to original fastled version
}

// Gradient functions for 1D, 2D and 3D Perlin noise  note: forcing inline produces smaller code and makes it 3x faster!
static inline __attribute__((always_inline)) int32_t gradient1D(uint32_t x0, int32_t dx) {
  uint32_t h = x0 * 0x27D4EB2D;
  h ^= h >> 15;
  h *= 0x92C3412B;
  h ^= h >> 13;
  h ^= h >> 7;
  return (hashToGradient(h) * dx) >> PERLIN_SHIFT;
}

static inline __attribute__((always_inline)) int32_t gradient2D(uint32_t x0, int32_t dx, uint32_t y0, int32_t dy) {
  uint32_t h = (x0 * 0x27D4EB2D) ^ (y0 * 0xB5297A4D);
  h ^= h >> 15;
  h *= 0x92C3412B;
  h ^= h >> 13;
  return (hashToGradient(h) * dx + hashToGradient(h>>PERLIN_SHIFT) * dy) >> (1 + PERLIN_SHIFT);
}

static inline __attribute__((always_inline)) int32_t gradient3D(uint32_t x0, int32_t dx, uint32_t y0, int32_t dy, uint32_t z0, int32_t dz) {
  // fast and good entropy hash from corner coordinates
  uint32_t h = (x0 * 0x27D4EB2D) ^ (y0 * 0xB5297A4D) ^ (z0 * 0x1B56C4E9);
  h ^= h >> 15;
  h *= 0x92C3412B;
  h ^= h >> 13;
  return ((hashToGradient(h) * dx + hashToGradient(h>>(1+PERLIN_SHIFT)) * dy + hashToGradient(h>>(1 + 2*PERLIN_SHIFT)) * dz) * 85) >> (8 + PERLIN_SHIFT); // scale to 16bit, x*85 >> 8 = x/3
}

// fast cubic smoothstep: t*(3 - 2t²), optimized for fixed point, scaled to avoid overflows
static uint32_t smoothstep(const uint32_t t) {
  uint32_t t_squared = (t * t) >> 16;
  uint32_t factor = (3 << 16) - ((t << 1));
  return (t_squared * factor) >> 18; // scale to avoid overflows and give best resolution
}

// simple linear interpolation for fixed-point values, scaled for perlin noise use
static inline int32_t lerpPerlin(int32_t a, int32_t b, int32_t t) {
    return a + (((b - a) * t) >> 14); // match scaling with smoothstep to yield 16.16bit values
}

// 1D Perlin noise function that returns a value in range of -24691 to 24689
int32_t IRAM_ATTR_YN perlin1D_raw(uint32_t x, bool is16bit) {
  // integer and fractional part coordinates
  int32_t x0 = x >> 16;
  int32_t x1 = x0 + 1;
  if(is16bit) x1 = x1 & 0xFF; // wrap back to zero at 0xFF instead of 0xFFFF

  int32_t dx0 = x & 0xFFFF;
  int32_t dx1 = dx0 - 0x10000;
  // gradient values for the two corners
  int32_t g0 = gradient1D(x0, dx0);
  int32_t g1 = gradient1D(x1, dx1);
  // interpolate and smooth function
  int32_t tx = smoothstep(dx0);
  int32_t noise = lerpPerlin(g0, g1, tx);
  return noise;
}

// 2D Perlin noise function that returns a value in range of -20633 to 20629
int32_t IRAM_ATTR_YN perlin2D_raw(uint32_t x, uint32_t y, bool is16bit) {
  int32_t x0 = x >> 16;
  int32_t y0 = y >> 16;
  int32_t x1 = x0 + 1;
  int32_t y1 = y0 + 1;

  if(is16bit) {
    x1 = x1 & 0xFF; // wrap back to zero at 0xFF instead of 0xFFFF
    y1 = y1 & 0xFF;
  }

  int32_t dx0 = x & 0xFFFF;
  int32_t dy0 = y & 0xFFFF;
  int32_t dx1 = dx0 - 0x10000;
  int32_t dy1 = dy0 - 0x10000;

  int32_t g00 = gradient2D(x0, dx0, y0, dy0);
  int32_t g10 = gradient2D(x1, dx1, y0, dy0);
  int32_t g01 = gradient2D(x0, dx0, y1, dy1);
  int32_t g11 = gradient2D(x1, dx1, y1, dy1);

  uint32_t tx = smoothstep(dx0);
  uint32_t ty = smoothstep(dy0);

  int32_t nx0 = lerpPerlin(g00, g10, tx);
  int32_t nx1 = lerpPerlin(g01, g11, tx);

  int32_t noise = lerpPerlin(nx0, nx1, ty);
  return noise;
}

// 3D Perlin noise function that returns a value in range of -16788 to 16381
int32_t IRAM_ATTR_YN perlin3D_raw(uint32_t x, uint32_t y, uint32_t z, bool is16bit) {
  int32_t x0 = x >> 16;
  int32_t y0 = y >> 16;
  int32_t z0 = z >> 16;
  int32_t x1 = x0 + 1;
  int32_t y1 = y0 + 1;
  int32_t z1 = z0 + 1;

  if(is16bit) {
    x1 = x1 & 0xFF; // wrap back to zero at 0xFF instead of 0xFFFF
    y1 = y1 & 0xFF;
    z1 = z1 & 0xFF;
  }

  int32_t dx0 = x & 0xFFFF;
  int32_t dy0 = y & 0xFFFF;
  int32_t dz0 = z & 0xFFFF;
  int32_t dx1 = dx0 - 0x10000;
  int32_t dy1 = dy0 - 0x10000;
  int32_t dz1 = dz0 - 0x10000;

  int32_t g000 = gradient3D(x0, dx0, y0, dy0, z0, dz0);
  int32_t g001 = gradient3D(x0, dx0, y0, dy0, z1, dz1);
  int32_t g010 = gradient3D(x0, dx0, y1, dy1, z0, dz0);
  int32_t g011 = gradient3D(x0, dx0, y1, dy1, z1, dz1);
  int32_t g100 = gradient3D(x1, dx1, y0, dy0, z0, dz0);
  int32_t g101 = gradient3D(x1, dx1, y0, dy0, z1, dz1);
  int32_t g110 = gradient3D(x1, dx1, y1, dy1, z0, dz0);
  int32_t g111 = gradient3D(x1, dx1, y1, dy1, z1, dz1);

  uint32_t tx = smoothstep(dx0);
  uint32_t ty = smoothstep(dy0);
  uint32_t tz = smoothstep(dz0);

  int32_t nx0 = lerpPerlin(g000, g100, tx);
  int32_t nx1 = lerpPerlin(g010, g110, tx);
  int32_t nx2 = lerpPerlin(g001, g101, tx);
  int32_t nx3 = lerpPerlin(g011, g111, tx);
  int32_t ny0 = lerpPerlin(nx0, nx1, ty);
  int32_t ny1 = lerpPerlin(nx2, nx3, ty);

  int32_t noise = lerpPerlin(ny0, ny1, tz);
  return noise;
}

// scaling functions for fastled replacement
uint16_t perlin16(uint32_t x) {
  return ((perlin1D_raw(x) * 1159) >> 10) + 32803; //scale to 16bit and offset (fastled range: about 4838 to 60766)
}

uint16_t perlin16(uint32_t x, uint32_t y) {
 return ((perlin2D_raw(x, y) * 1537) >> 10) + 32725; //scale to 16bit and offset (fastled range: about 1748 to 63697)
}

uint16_t perlin16(uint32_t x, uint32_t y, uint32_t z) {
  return ((perlin3D_raw(x, y, z) * 1731) >> 10) + 33147; //scale to 16bit and offset (fastled range: about 4766 to 60840)
}

uint8_t perlin8(uint16_t x) {
  return (((perlin1D_raw((uint32_t)x << 8, true) * 1353) >> 10) + 32769) >> 8; //scale to 16 bit, offset, then scale to 8bit
}

uint8_t perlin8(uint16_t x, uint16_t y) {
  return (((perlin2D_raw((uint32_t)x << 8, (uint32_t)y << 8, true) * 1620) >> 10) + 32771) >> 8; //scale to 16 bit, offset, then scale to 8bit
}

uint8_t perlin8(uint16_t x, uint16_t y, uint16_t z) {
  return (((perlin3D_raw((uint32_t)x << 8, (uint32_t)y << 8, (uint32_t)z << 8, true) * 2015) >> 10) + 33168) >> 8; //scale to 16 bit, offset, then scale to 8bit
}
