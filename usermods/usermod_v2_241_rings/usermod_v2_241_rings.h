#pragma once

#include "wled.h"
//========================================================================================================================

static const char _data_FX_mode_Rings[] PROGMEM = "Rings - Rings@Speed,Jump,,,,Inward;;!,!;1;";
// static const char _data_FX_mode_SimpleRings[] PROGMEM = "Rings - Simple@Speed,Jump,,,,;;!,!;1;";
static const char _data_FX_mode_RandomFlow[] PROGMEM = "Rings - Random Flow@Speed;!;;1";
static const char _data_FX_mode_AudioRings[] PROGMEM = "Rings - AudioRings@Speed,,,,,Fade,Inward;;!;1f;pal=11,sx=255";

#define ringCount 9 // total Number of Rings
#define RINGS 9

//Map rings on disk to indicies.
//This is where all the magic happens.
//Each represents one of the concentric rings.
uint8_t ringMap[ringCount][2] = {
  {0, 0},     //0 Center Point
  {1, 8},     //1
  {9, 20},   //2
  {21, 36},   //3
  {37, 60},   //4
  {61, 92},   //5
  {93, 132},  //6
  {133, 180}, //7
  {181, 240}, //8 Outer Ring
};

void setRing(int ring, CRGB colour) {
  for (int i = ringMap[ring][0]; i <= ringMap[ring][1]; i++) {
    SEGMENT.setPixelColor(i, colour);
  }
}

void setRingFromFtt(int index, int ring) {
  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  uint8_t *fftResult = (uint8_t*)um_data->u_data[2];

  uint8_t val = fftResult[map(index, 0, 6, 0, NUM_GEQ_CHANNELS)];
  // Visualize leds to the beat
  CRGB color =  SEGMENT.color_from_palette(val, false, false, 0);
  color.nscale8_video(val);
  setRing(ring, color);
}


uint16_t mode_Rings() {
  
  int JUMP = map(SEGMENT.intensity, 0, 255, 1,50); // TODO: set range
  bool INWARD = SEGMENT.check1;

  static uint8_t hue[RINGS];

  if (SEGENV.call == 0) {
    for (int r = 0; r < RINGS; r++) {
      hue[r] = random(0, 255);
    }
  }

  for (int r = 0; r < RINGS; r++) {
    setRing(r, SEGMENT.color_from_palette(hue[r], false, false, 0));
  }
  delay(map(SEGMENT.speed, 0, 255, 200, 0)); // FIX
  if (INWARD) {
    for (int r = 0; r < RINGS; r++) {
      hue[(r - 1)] = hue[r]; // set ring one less to that of the outer
    }
    hue[(RINGS - 1)] += JUMP;
  }
  else {
    for (int r = (RINGS - 1); r >= 1; r--) {
      hue[r] = hue[(r - 1)]; // set this ruing based on the inner
    }
    hue[0] += JUMP;
  }
  return FRAMETIME; // TODO
}

// uint16_t mode_SimpleRings() {
//   int JUMP = map(SEGMENT.intensity, 0, 255, 1,50); // TODO: set range
//   static uint8_t j;
//   for (int r = 0; r < RINGS; r++) {
//     setRing(r, SEGMENT.color_from_palette(j + (r * JUMP), false, false, 0));
//   }
//   j += JUMP;
// //   FastLED.delay(SPEED);
//   delay(map(SEGMENT.speed, 0, 255, 200, 0)); // FIX
//   return FRAMETIME_FIXED_SLOW; // TODO
// }


uint16_t mode_RandomFlow() {
  static int hue[RINGS];
  hue[0] = random(0, 255);
  for (int r = 0; r < RINGS; r++) {
    setRing(r, CHSV(hue[r], 255, 255));
  }
  for (int r = (RINGS - 1); r >= 1; r--) {
    hue[r] = hue[(r - 1)]; // set this ring based on the inner
  }
  delay(map(SEGMENT.speed, 0, 255, 200, 0)); // FIX
  return FRAMETIME_FIXED_SLOW; // TODO
}


uint16_t mode_AudioRings() {

  bool newReading = false; // TODO  return FRAMETIME_FIXED_SLOW; // TODO

  bool INWARD = SEGMENT.check2;

  uint8_t secondHand = micros()/(256-SEGMENT.speed)/500+1 % 64;
  if((SEGMENT.speed > 254) || (SEGENV.aux0 != secondHand)) {   // WLEDMM allow run run at full speed
    SEGENV.aux0 = secondHand;
    newReading = true;
  }

  if(newReading) {
    um_data_t *um_data;
    if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
      // add support for no audio
      um_data = simulateSound(SEGMENT.soundSim);
    }
    uint8_t *fftResult = (uint8_t*)um_data->u_data[2];

    for (int i = 0; i < 7; i++) {

      uint8_t val;
      if(INWARD) {
        val = fftResult[(i*2)];
      }
      else {
        int b = 14 -(i*2);
        val = fftResult[b];
      }
  
      // Visualize leds to the beat
      CRGB color = SEGMENT.color_from_palette(val, false, false, 0, val);
//      CRGB color = ColorFromPalette(currentPalette, val, 255, currentBlending);
    if(SEGMENT.check1) color.nscale8_video(val);
      setRing(i, color);
//        setRingFromFtt((i * 2), i); 
    }

    setRingFromFtt(2, 7); // set outer ring to base
    setRingFromFtt(0, 8); // set outer ring to base

  }
  return 0;
}


class Rings241Usermod : public Usermod {

  public:

    Rings241Usermod(const char *name, bool enabled):Usermod(name, enabled) {} //WLEDMM
	

    void setup() {
		
		if(!enabled) return;

		strip.addEffect(250, &mode_Rings, _data_FX_mode_Rings);
		// strip.addEffect(251, &mode_SimpleRings, _data_FX_mode_SimpleRings);
		strip.addEffect(252, &mode_RandomFlow, _data_FX_mode_RandomFlow);
		strip.addEffect(253, &mode_AudioRings, _data_FX_mode_AudioRings);

		initDone = true;
    }

	void loop() {}

    uint16_t getId()
    {
      return USERMOD_ID_241RINGS;
    }

};



