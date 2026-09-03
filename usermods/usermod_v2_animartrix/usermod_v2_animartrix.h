#pragma once

#include "wled.h"
#include "colors.h" // include CHSV32 class by @dedehai

#ifdef _MoonModules_WLED_
	// WLEDMM: use faster math approximations - up to 40% faster
	static inline float my_floor_t(float x) {
  	bool neg = x < 0.0f;
		int val = x;
  	if (neg) val--;
  	return val;
	}
	static inline float my_fmod_t(float num, float denom) {
  	int tquot = num / denom;
  	float res = num - tquot * denom;
  	return res;
	}
	// #undef fmodf  //don't replace fmodf - the integer approximation version leads to effect freak-out (radial osciallator) after a few hours.
	// #define fmodf(num, denum) my_fmod_t(num, denum)
	#undef floorf
	#define floorf(num) my_floor_t(num)
	#define sinf(angle) sin_approx(angle)
	#define cosf(angle) cos_approx(angle)
	#define tanf(angle) tan_approx(angle)

	// WLEDMM specific speedups for segment access
	#ifdef WLEDMM_FASTPATH
		#undef SEGMENT
		#undef SEGENV
		#define SEGMENT (*strip._currentSeg) // saves us many calls to strip._segments[strip.getCurrSegmentId()]
		#define SEGENV SEGMENT
	#endif
#endif
// WLEDMM end

// softhack007: workaround for ICE (internal compiler error) when compiling with new framework and "-O2":

/* 
	wled00/../usermods/usermod_v2_animartrix/usermod_v2_animartrix.h: In function 'uint16_t mode_Waves()':
	wled00/../usermods/usermod_v2_animartrix/usermod_v2_animartrix.h:367:1: error: insn does not satisfy its constraints:
 	}
 	^
	(insn 811 738 824 24 (set (reg/v:SF 19 f0 [orig:69 result ] [69])
        	(mem/u/c:SF (symbol_ref/u:SI ("*.LC1657") [flags 0x2]) [0  S4 A32])) ".pio/libdeps/my_esp32_16MB_V4_S/animartrix/ANIMartRIX.h":372 47 {movsf_internal}
     	(nil))
	during RTL pass: postreload
	wled00/../usermods/usermod_v2_animartrix/usermod_v2_animartrix.h:367:1: internal compiler error: in extract_constrain_insn, at recog.c:2210
	libbacktrace could not find executable to open
	Please submit a full bug report,
	with preprocessed source if appropriate.
	See <https://gcc.gnu.org/bugs/> for instructions.
*/

#if defined(ARDUINO_ARCH_ESP32) && defined(ESP_IDF_VERSION)
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 0)
  // this pragma temporarily raises gcc optimization level to "-O3", to avoid internal error conditions
	// -fsingle-precision-constant = all literals are float (default is "double" = very slow)
	// -ffast-math = additional math optimizations that may reduce accurcy
  #pragma GCC push_options
  #pragma GCC optimize ("O3,single-precision-constant,fast-math")
#endif
#endif

#define ANIMartRIX_NO_SERPENTINE  // disable serpentine calculation code - already done by WLED so never needed

#include <ANIMartRIX.h>

#warning WLEDMM usermod: CC BY-NC 3.0 licensed effects by Stefan Petrick, include this usermod only if you accept the terms!
//========================================================================================================================


// Combined effects with custom parameters (effects that share a _base method in ANIMartRIX)
static const char _data_FX_mode_Module_Experiment810[] PROGMEM = "Y💡Module_Experiment810 ☾@Speed,,W,,, HSV Color,Gamma Correction;;1;2;c1=128,o1=0,o2=0";  // c1: w 0.3-1.0, o1: use_hsv
static const char _data_FX_mode_Module_Experiment9[] PROGMEM = "Y💡Module_Experiment9 ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Module_Experiment7[] PROGMEM = "Y💡Module_Experiment7 ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Module_Experiment6[] PROGMEM = "Y💡Module_Experiment6 ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Module_Experiment5[] PROGMEM = "Y💡Module_Experiment5 ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Module_Experiment4[] PROGMEM = "Y💡Module_Experiment4 ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Zoom2[] PROGMEM = "Y💡Zoom2 ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Module_Experiment23[] PROGMEM = "Y💡Module_Experiment23 ☾@Speed,,Dist Offset,Dist Scale,,,Gamma Correction;;1;2;c1=128,c2=128,o2=0";  // c1: dist_offset, c2: dist_scale
static const char _data_FX_mode_Module_Experiment1[] PROGMEM = "Y💡Module_Experiment1 ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Parametric_Water[] PROGMEM = "Y💡Parametric_Water ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Water[] PROGMEM = "Y💡Water ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Complex_Kaleido_6[] PROGMEM = "Y💡Complex_Kaleido_6 ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Complex_Kaleido_5[] PROGMEM = "Y💡Complex_Kaleido_5 ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Complex_Kaleido_4[] PROGMEM = "Y💡Complex_Kaleido_4 ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Complex_Kaleido_3[] PROGMEM = "Y💡Complex_Kaleido_3 ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Complex_Kaleido12[] PROGMEM = "Y💡Complex_Kaleido12 ☾@Speed,,Size,,,, Gamma Correction;;1;2;c1=128,o2=0";  // c1: size 0.5-1.0
static const char _data_FX_mode_SM910[] PROGMEM = "Y💡SM910 ☾@Speed,,Scale,,, Variant,Gamma Correction;;1;2;c1=128,o1=0,o2=0";  // c1: scale 0.6-1.0, o1: variant10
static const char _data_FX_mode_SM8[] PROGMEM = "Y💡SM8 ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_SM7[] PROGMEM = "Y💡SM7 ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_SM56[] PROGMEM = "Y💡SM56 ☾@Speed,,Zoom,,, Variant,Gamma Correction;;1;2;c1=128,o1=0,o2=0";  // c1: s 0.7-1.0, o1: variant6
static const char _data_FX_mode_SM4[] PROGMEM = "Y💡SM4 ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_SM3[] PROGMEM = "Y💡SM3 ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_SM2[] PROGMEM = "Y💡SM2 ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_SM1[] PROGMEM = "Y💡SM1 ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Big_Caleido[] PROGMEM = "Y💡Big_Caleido ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_RGB_Blobs45[] PROGMEM = "Y💡RGB_Blobs45 ☾@Speed,,Scale,,,, Gamma Correction;;1;2;c1=128,o2=0";  // c1: scale 0.05-0.1
static const char _data_FX_mode_RGB_Blobs23[] PROGMEM = "Y💡RGB_Blobs23 ☾@Speed,,,,,Variant,Gamma Correction;;1;2;o1=0,o2=0";  // o1: variant3
static const char _data_FX_mode_RGB_Blobs[] PROGMEM = "Y💡RGB_Blobs ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Polar_Waves[] PROGMEM = "Y💡Polar_Waves ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Slow_Fade[] PROGMEM = "Y💡Slow_Fade ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Zoom[] PROGMEM = "Y💡Zoom ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Hot_Blob[] PROGMEM = "Y💡Hot_Blob ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Spiralus2[] PROGMEM = "Y💡Spiralus2 ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Spiralus[] PROGMEM = "Y💡Spiralus ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Yves[] PROGMEM = "Y💡Yves ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Scaledemo1[] PROGMEM = "Y💡Scaledemo1 ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Lava1[] PROGMEM = "Y💡Lava1 ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Caleido3[] PROGMEM = "Y💡Caleido3 ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Caleido2[] PROGMEM = "Y💡Caleido2 ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Caleido1[] PROGMEM = "Y💡Caleido1 ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Distance_Experiment[] PROGMEM = "Y💡Distance_Experiment ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Center_Field[] PROGMEM = "Y💡Center_Field ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Waves[] PROGMEM = "Y💡Waves ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Chasing_Spirals[] PROGMEM = "Y💡Chasing_Spirals ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";
static const char _data_FX_mode_Rotating_Blob[] PROGMEM = "Y💡Rotating_Blob ☾@Speed,,,,,,Gamma Correction;;1;2;o2=0";

// local float map helper (same as in FX.cpp but needed here since that one is static)
static float animartrix_mapf(float x, float in_min, float in_max, float out_min, float out_max) {
	if (in_max == in_min) return out_min;
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

class ANIMartRIXMod:public ANIMartRIX {
	private:
	bool use_gamma = false;
	public:
	void initEffect() {
	  if ((SEGENV.call == 0) || (SEGMENT.virtualWidth() != num_x) || (SEGMENT.virtualHeight() != num_y)) {
		  init(SEGMENT.virtualWidth(), SEGMENT.virtualHeight(), false);
	  }
	  float speedFactor = 1.0;
	  if (SEGMENT.speed < 128) {
		  speedFactor = (float) map(SEGMENT.speed,   0, 127, 1, 100) / 100.0f;
	  } else {
		  speedFactor = (float) map(SEGMENT.speed, 128, 255, 10, 100) / 10.0f;
	  }
	  use_gamma = SEGENV.check2;
	  setSpeedFactor(speedFactor);
	}

	void setPixelColor(int x, int y, rgb pixel) override {
		uint8_t colR, colG, colB;
		if (use_gamma) {
			colR = gamma8(pixel.red); colG = gamma8(pixel.green); colB = gamma8(pixel.blue);
		} else {
			colR = pixel.red; colG = pixel.green; colB = pixel.blue;
		}
		SEGMENT.setPixelColorXY(x, y, RGBW32(colR,colG,colB,0));
	}

	void setPixelColor(int index, rgb pixel) override {
		uint8_t colR, colG, colB;
		if (use_gamma) {
			colR = gamma8(pixel.red); colG = gamma8(pixel.green); colB = gamma8(pixel.blue);
		} else {
			colR = pixel.red; colG = pixel.green; colB = pixel.blue;
		}
		// get x and y, so we can us setPixelColorXY() - faster in WLEDMM
		int x = index % num_x;
		int y = index / num_x;
		SEGMENT.setPixelColorXY(x,y, RGBW32(colR,colG,colB,0));
  	}

	// Add any extra custom effects not part of the ANIMartRIX libary here
};
ANIMartRIXMod anim;

uint16_t mode_Module_Experiment810() {
	anim.initEffect();
	// c1 maps 0-255 to w range 0.3-1.0, o1 selects HSV color mode
	float w = animartrix_mapf(SEGMENT.custom1, 0, 255, 0.3f, 1.0f);
	bool use_hsv = SEGMENT.check1;
	anim.Module_Experiment810_base(w, use_hsv);
	return FRAMETIME;
}
uint16_t mode_Module_Experiment9() { 
	anim.initEffect(); 
	anim.Module_Experiment9();
	return FRAMETIME;
}
uint16_t mode_Module_Experiment7() { 
	anim.initEffect(); 
	anim.Module_Experiment7();
	return FRAMETIME;
}
uint16_t mode_Module_Experiment6() { 
	anim.initEffect(); 
	anim.Module_Experiment6();
	return FRAMETIME;
}
uint16_t mode_Module_Experiment5() { 
	anim.initEffect(); 
	anim.Module_Experiment5();
	return FRAMETIME;
}
uint16_t mode_Module_Experiment4() { 
	anim.initEffect(); 
	anim.Module_Experiment4();
	return FRAMETIME;
}
uint16_t mode_Zoom2() { 
	anim.initEffect(); 
	anim.Zoom2();
	return FRAMETIME;
}
uint16_t mode_Module_Experiment23() {
	anim.initEffect();
	// c1 maps 0-255 to dist_offset 12-16, c2 maps 0-255 to dist_scale 4-16
	float dist_offset = animartrix_mapf(SEGMENT.custom1, 0, 255, 12.0f, 16.0f);
	float dist_scale = animartrix_mapf(SEGMENT.custom2, 0, 255, 4.0f, 16.0f);
	// speed derived from dist_offset ratio, dir_idx: 0 for high offset, 3 for low
	float speed = animartrix_mapf(SEGMENT.custom1, 0, 255, 0.01f, 0.02f);
	int dir_idx = (SEGMENT.custom1 < 128) ? 3 : 0;
	anim.Module_Experiment23_base(speed, dist_offset, dist_scale, dir_idx);
	return FRAMETIME;
}
uint16_t mode_Module_Experiment1() { 
	anim.initEffect(); 
	anim.Module_Experiment1();
	return FRAMETIME;
}
uint16_t mode_Parametric_Water() { 
	anim.initEffect(); 
	anim.Parametric_Water();
	return FRAMETIME;
}
uint16_t mode_Water() { 
	anim.initEffect(); 
	anim.Water();
	return FRAMETIME;
}
uint16_t mode_Complex_Kaleido_6() { 
	anim.initEffect(); 
	anim.Complex_Kaleido_6();
	return FRAMETIME;
}
uint16_t mode_Complex_Kaleido_5() { 
	anim.initEffect(); 
	anim.Complex_Kaleido_5();
	return FRAMETIME;
}
uint16_t mode_Complex_Kaleido_4() { 
	anim.initEffect(); 
	anim.Complex_Kaleido_4();
	return FRAMETIME;
}
uint16_t mode_Complex_Kaleido_3() { 
	anim.initEffect(); 
	anim.Complex_Kaleido_3();
	return FRAMETIME;
}
uint16_t mode_Complex_Kaleido12() {
	anim.initEffect();
	// c1 maps 0-255 to size 0.5-1.0
	float size = animartrix_mapf(SEGMENT.custom1, 0, 255, 0.5f, 1.0f);
	anim.Complex_Kaleido_12_base(size);
	return FRAMETIME;
}
uint16_t mode_SM910() {
	anim.initEffect();
	// c1 maps 0-255 to scale 0.6-1.0, o1 selects variant10
	float scale = animartrix_mapf(SEGMENT.custom1, 0, 255, 0.6f, 1.0f);
	bool variant10 = SEGMENT.check1;
	float speed = variant10 ? 0.006f : 0.005f;
	anim.SM910_base(speed, scale, variant10);
	return FRAMETIME;
}
uint16_t mode_SM8() { 
	anim.initEffect(); 
	anim.SM8();
	return FRAMETIME;
}
// uint16_t mode_SM7() { 
//	anim.initEffect(); 
// 	anim.SM7();
//
//	return FRAMETIME;
// }
uint16_t mode_SM56() {
	anim.initEffect();
	// c1 maps 0-255 to s 0.7-1.0, o1 selects variant6
	float s = animartrix_mapf(SEGMENT.custom1, 0, 255, 0.7f, 1.0f);
	bool variant6 = SEGMENT.check1;
	anim.SM56_base(s, variant6);
	return FRAMETIME;
}
uint16_t mode_SM4() { 
	anim.initEffect(); 
	anim.SM4();
	return FRAMETIME;
}
uint16_t mode_SM3() { 
	anim.initEffect(); 
	anim.SM3();
	return FRAMETIME;
}
uint16_t mode_SM2() { 
	anim.initEffect(); 
	anim.SM2();
	return FRAMETIME;
}
uint16_t mode_SM1() { 
	anim.initEffect(); 
	anim.SM1();
	return FRAMETIME;
}
uint16_t mode_Big_Caleido() { 
	anim.initEffect(); 	
	anim.Big_Caleido();
	return FRAMETIME;
}
uint16_t mode_RGB_Blobs45() {
	anim.initEffect();
	// c1 maps 0-255 to scale 0.05-0.1
	float scale = animartrix_mapf(SEGMENT.custom1, 0, 255, 0.05f, 0.1f);
	anim.RGB_Blobs45_base(scale);
	return FRAMETIME;
}
uint16_t mode_RGB_Blobs23() {
	anim.initEffect();
	// o1 selects variant3 (different dist offset and color mapping)
	bool variant3 = SEGMENT.check1;
	anim.RGB_Blobs23_base(variant3);
	return FRAMETIME;
}
uint16_t mode_RGB_Blobs() { 
	anim.initEffect(); 
	anim.RGB_Blobs();
	return FRAMETIME;
}
uint16_t mode_Polar_Waves() { 
	anim.initEffect(); 
	anim.Polar_Waves();
	return FRAMETIME;
}
uint16_t mode_Slow_Fade() { 
	anim.initEffect(); 
	anim.Slow_Fade();
	return FRAMETIME;
}
uint16_t mode_Zoom() { 
	anim.initEffect(); 
	anim.Zoom();
	return FRAMETIME;
}
uint16_t mode_Hot_Blob() { 
	anim.initEffect(); 
	anim.Hot_Blob();
	return FRAMETIME;
}
uint16_t mode_Spiralus2() { 
	anim.initEffect(); 
	anim.Spiralus2();
	return FRAMETIME;
}
uint16_t mode_Spiralus() { 
	anim.initEffect(); 
	anim.Spiralus();
	return FRAMETIME;
}
uint16_t mode_Yves() { 
	anim.initEffect(); 
	anim.Yves();
	return FRAMETIME;
}
uint16_t mode_Scaledemo1() { 
	anim.initEffect(); 
	anim.Scaledemo1();
	return FRAMETIME;
}
uint16_t mode_Lava1() { 
	anim.initEffect(); 
	anim.Lava1();
	return FRAMETIME;
}
uint16_t mode_Caleido3() { 
	anim.initEffect(); 
	anim.Caleido3();
	return FRAMETIME;
}
uint16_t mode_Caleido2() { 
	anim.initEffect(); 
	anim.Caleido2();
	return FRAMETIME;
}
uint16_t mode_Caleido1() { 
	anim.initEffect(); 
	anim.Caleido1();
	return FRAMETIME;
}
uint16_t mode_Distance_Experiment() { 
	anim.initEffect(); 
	anim.Distance_Experiment();
	return FRAMETIME;
}
uint16_t mode_Center_Field() { 
	anim.initEffect(); 
	anim.Center_Field();
	return FRAMETIME;
}
uint16_t mode_Waves() { 
	anim.initEffect(); 
	anim.Waves();
	return FRAMETIME;
}
uint16_t mode_Chasing_Spirals() { 
	anim.initEffect(); 
	anim.Chasing_Spirals();
	return FRAMETIME;
}
uint16_t mode_Rotating_Blob() { 
	anim.initEffect(); 
	anim.Rotating_Blob();
	return FRAMETIME;
}


class AnimartrixUsermod : public Usermod {

  public:
#ifdef _MoonModules_WLED_
    AnimartrixUsermod(const char *name, bool enabled):Usermod(name, enabled) {} //WLEDMM
#endif

    void setup() override {
		
		if(!enabled) return;

      strip.addEffect(203, &mode_Module_Experiment810, _data_FX_mode_Module_Experiment810);
      strip.addEffect(204, &mode_Module_Experiment9, _data_FX_mode_Module_Experiment9);
      strip.addEffect(206, &mode_Module_Experiment7, _data_FX_mode_Module_Experiment7);
      strip.addEffect(207, &mode_Module_Experiment6, _data_FX_mode_Module_Experiment6);
      strip.addEffect(208, &mode_Module_Experiment5, _data_FX_mode_Module_Experiment5);
      strip.addEffect(209, &mode_Module_Experiment4, _data_FX_mode_Module_Experiment4);
      strip.addEffect(210, &mode_Zoom2, _data_FX_mode_Zoom2);
      strip.addEffect(211, &mode_Module_Experiment23, _data_FX_mode_Module_Experiment23);
      strip.addEffect(213, &mode_Module_Experiment1, _data_FX_mode_Module_Experiment1);
      strip.addEffect(214, &mode_Parametric_Water, _data_FX_mode_Parametric_Water);
      strip.addEffect(215, &mode_Water, _data_FX_mode_Water);
      strip.addEffect(216, &mode_Complex_Kaleido_6, _data_FX_mode_Complex_Kaleido_6);
      strip.addEffect(217, &mode_Complex_Kaleido_5, _data_FX_mode_Complex_Kaleido_5);
      strip.addEffect(218, &mode_Complex_Kaleido_4, _data_FX_mode_Complex_Kaleido_4);
      strip.addEffect(219, &mode_Complex_Kaleido_3, _data_FX_mode_Complex_Kaleido_3);
      strip.addEffect(220, &mode_Complex_Kaleido12, _data_FX_mode_Complex_Kaleido12);
      strip.addEffect(222, &mode_SM910, _data_FX_mode_SM910);
      strip.addEffect(224, &mode_SM8, _data_FX_mode_SM8);
      // strip.addEffect(225, &mode_SM7, _data_FX_mode_SM7);
      strip.addEffect(226, &mode_SM56, _data_FX_mode_SM56);
      strip.addEffect(228, &mode_SM4, _data_FX_mode_SM4);
      strip.addEffect(229, &mode_SM3, _data_FX_mode_SM3);
      strip.addEffect(230, &mode_SM2, _data_FX_mode_SM2);
      strip.addEffect(231, &mode_SM1, _data_FX_mode_SM1);
      strip.addEffect(232, &mode_Big_Caleido, _data_FX_mode_Big_Caleido);
      strip.addEffect(233, &mode_RGB_Blobs45, _data_FX_mode_RGB_Blobs45);
      strip.addEffect(235, &mode_RGB_Blobs23, _data_FX_mode_RGB_Blobs23);
      strip.addEffect(237, &mode_RGB_Blobs, _data_FX_mode_RGB_Blobs);
      strip.addEffect(238, &mode_Polar_Waves, _data_FX_mode_Polar_Waves);
      strip.addEffect(239, &mode_Slow_Fade, _data_FX_mode_Slow_Fade);
      strip.addEffect(240, &mode_Zoom, _data_FX_mode_Zoom);
      strip.addEffect(241, &mode_Hot_Blob, _data_FX_mode_Hot_Blob);
      strip.addEffect(242, &mode_Spiralus2, _data_FX_mode_Spiralus2);
      strip.addEffect(243, &mode_Spiralus, _data_FX_mode_Spiralus);
      strip.addEffect(244, &mode_Yves, _data_FX_mode_Yves);
      strip.addEffect(245, &mode_Scaledemo1, _data_FX_mode_Scaledemo1);
      strip.addEffect(246, &mode_Lava1, _data_FX_mode_Lava1);
      strip.addEffect(247, &mode_Caleido3, _data_FX_mode_Caleido3);
      strip.addEffect(248, &mode_Caleido2, _data_FX_mode_Caleido2);
      strip.addEffect(249, &mode_Caleido1, _data_FX_mode_Caleido1);
      strip.addEffect(250, &mode_Distance_Experiment, _data_FX_mode_Distance_Experiment);
      strip.addEffect(251, &mode_Center_Field, _data_FX_mode_Center_Field);
      strip.addEffect(252, &mode_Waves, _data_FX_mode_Waves);
      strip.addEffect(253, &mode_Chasing_Spirals, _data_FX_mode_Chasing_Spirals);
      strip.addEffect(254, &mode_Rotating_Blob, _data_FX_mode_Rotating_Blob);

      initDone = true;
    }

    void loop() override {
    #if 0  // not needed
       if (!enabled || strip.isUpdating()) return;

      // do your magic here
      if (millis() - lastTime > 1000) {
        //USER_PRINTLN("I'm alive!");
        lastTime = millis();
      }
    #else
			return;
    #endif
    }

    void addToJsonInfo(JsonObject& root) override
    {
	  if(!enabled) return;
      char myStringBuffer[16]; // buffer for snprintf()
      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

      JsonArray infoArr = user.createNestedArray(FPSTR(_name));

      String uiDomString = F("Animartrix requires the Creative Commons Attribution License CC BY-NC 3.0");
      infoArr.add(uiDomString);
	}

    uint16_t getId() override
    {
      return USERMOD_ID_ANIMARTRIX;
    }

};


#if defined(ARDUINO_ARCH_ESP32) && defined(ESP_IDF_VERSION)
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 0)
  // restore original gcc optimization level
  #pragma GCC pop_options
#endif
#endif

#ifdef _MoonModules_WLED_
// WLEDMM cleanup
#undef fmodf
#undef floorf
#undef sinf
#undef cosf
#undef tanf
#endif
