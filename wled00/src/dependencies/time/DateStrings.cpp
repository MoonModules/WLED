/* DateStrings.cpp
 * Definitions for date strings for use with the Time library
 *
 * Updated for Arduino 1.5.7 18 July 2014
 *
 * No memory is consumed in the sketch if your code does not call any of the string methods
 * You can change the text of the strings, make sure the short strings are each exactly 3 characters 
 * the long strings can be any length up to the constant dt_MAX_STRING_LEN defined in TimeLib.h
 * 
 */

#if defined(__AVR__)
#include <avr/pgmspace.h>
#else
#if defined(ARDUINO_ARCH_ESP32)
#include <pgmspace.h>
#else
// for compatiblity with Arduino Due and Teensy 3.0 and maybe others?
#define PROGMEM
#define PGM_P  const char *
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#define pgm_read_word(addr) (*(const unsigned char **)(addr))
#define strcpy_P(dest, src) strcpy((dest), (src))
#endif
#endif
#include <string.h> // for strcpy_P or strcpy
#include "TimeLib.h"
 
// the short strings for each day or month must be exactly dt_SHORT_STR_LEN
#define dt_SHORT_STR_LEN  3 // the length of short strings

static char buffer[dt_MAX_STRING_LEN+1];  // must be big enough for longest string and the terminating null

static const char monthStr0[] PROGMEM = "";
static const char monthStr1[] PROGMEM = "January";
static const char monthStr2[] PROGMEM = "February";
static const char monthStr3[] PROGMEM = "March";
static const char monthStr4[] PROGMEM = "April";
static const char monthStr5[] PROGMEM = "May";
static const char monthStr6[] PROGMEM = "June";
static const char monthStr7[] PROGMEM = "July";
static const char monthStr8[] PROGMEM = "August";
static const char monthStr9[] PROGMEM = "September";
static const char monthStr10[] PROGMEM = "October";
static const char monthStr11[] PROGMEM = "November";
static const char monthStr12[] PROGMEM = "December";

static const PROGMEM char * const PROGMEM monthNames_P[] =
{
    monthStr0,monthStr1,monthStr2,monthStr3,monthStr4,monthStr5,monthStr6,
    monthStr7,monthStr8,monthStr9,monthStr10,monthStr11,monthStr12
};

static const char monthShortNames_P[] PROGMEM = "ErrJanFebMarAprMayJunJulAugSepOctNovDec";

static const char dayStr0[] PROGMEM = "Err";
static const char dayStr1[] PROGMEM = "Sunday";
static const char dayStr2[] PROGMEM = "Monday";
static const char dayStr3[] PROGMEM = "Tuesday";
static const char dayStr4[] PROGMEM = "Wednesday";
static const char dayStr5[] PROGMEM = "Thursday";
static const char dayStr6[] PROGMEM = "Friday";
static const char dayStr7[] PROGMEM = "Saturday";

static const PROGMEM char * const PROGMEM dayNames_P[] =
{
   dayStr0,dayStr1,dayStr2,dayStr3,dayStr4,dayStr5,dayStr6,dayStr7
};

static const char dayShortNames_P[] PROGMEM = "ErrSunMonTueWedThuFriSat";

/* functions to return date strings */

char* monthStr(uint8_t month)
{
    strcpy_P(buffer, (PGM_P)pgm_read_word(&(monthNames_P[month])));
    return buffer;
}

char* monthShortStr(uint8_t month)
{
   for (int i=0; i < dt_SHORT_STR_LEN; i++)      
      buffer[i] = pgm_read_byte(&(monthShortNames_P[i+ (month*dt_SHORT_STR_LEN)]));  
   buffer[dt_SHORT_STR_LEN] = 0;
   return buffer;
}

char* dayStr(uint8_t day) 
{
   strcpy_P(buffer, (PGM_P)pgm_read_word(&(dayNames_P[day])));
   return buffer;
}

char* dayShortStr(uint8_t day) 
{
   uint8_t index = day*dt_SHORT_STR_LEN;
   for (int i=0; i < dt_SHORT_STR_LEN; i++)      
      buffer[i] = pgm_read_byte(&(dayShortNames_P[index + i]));  
   buffer[dt_SHORT_STR_LEN] = 0; 
   return buffer;
}
