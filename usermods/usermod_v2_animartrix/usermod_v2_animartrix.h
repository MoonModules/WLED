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
	#undef fmodf
	#define fmodf(num, denum) my_fmod_t(num, denum)
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

#define ANIMARTRIX_UI_CONTROLS "Speed,HUE Change,,,,cycle HUE,boost Brightness,boost Contrast;;1;2;o2=0"

static const char _data_FX_mode_Module_Experiment10[] PROGMEM = "Y💡Module_Experiment10 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Module_Experiment9[] PROGMEM = "Y💡Module_Experiment9 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Module_Experiment8[] PROGMEM = "Y💡Module_Experiment8 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Module_Experiment7[] PROGMEM = "Y💡Module_Experiment7 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Module_Experiment6[] PROGMEM = "Y💡Module_Experiment6 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Module_Experiment5[] PROGMEM = "Y💡Module_Experiment5 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Module_Experiment4[] PROGMEM = "Y💡Module_Experiment4 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Zoom2[] PROGMEM = "Y💡Zoom2 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Module_Experiment3[] PROGMEM = "Y💡Module_Experiment3 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Module_Experiment2[] PROGMEM = "Y💡Module_Experiment2 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Module_Experiment1[] PROGMEM = "Y💡Module_Experiment1 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Parametric_Water[] PROGMEM = "Y💡Parametric_Water ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Water[] PROGMEM = "Y💡Water ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Complex_Kaleido_6[] PROGMEM = "Y💡Complex_Kaleido_6 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Complex_Kaleido_5[] PROGMEM = "Y💡Complex_Kaleido_5 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Complex_Kaleido_4[] PROGMEM = "Y💡Complex_Kaleido_4 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Complex_Kaleido_3[] PROGMEM = "Y💡Complex_Kaleido_3 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Complex_Kaleido_2[] PROGMEM = "Y💡Complex_Kaleido_2 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Complex_Kaleido[] PROGMEM = "Y💡Complex_Kaleido ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_SM10[] PROGMEM = "Y💡SM10 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_SM9[] PROGMEM = "Y💡SM9 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_SM8[] PROGMEM = "Y💡SM8 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_SM7[] PROGMEM = "Y💡SM7 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_SM6[] PROGMEM = "Y💡SM6 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_SM5[] PROGMEM = "Y💡SM5 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_SM4[] PROGMEM = "Y💡SM4 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_SM3[] PROGMEM = "Y💡SM3 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_SM2[] PROGMEM = "Y💡SM2 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_SM1[] PROGMEM = "Y💡SM1 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Big_Caleido[] PROGMEM = "Y💡Big_Caleido ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_RGB_Blobs5[] PROGMEM = "Y💡RGB_Blobs5 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_RGB_Blobs4[] PROGMEM = "Y💡RGB_Blobs4 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_RGB_Blobs3[] PROGMEM = "Y💡RGB_Blobs3 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_RGB_Blobs2[] PROGMEM = "Y💡RGB_Blobs2 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_RGB_Blobs[] PROGMEM = "Y💡RGB_Blobs ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Polar_Waves[] PROGMEM = "Y💡Polar_Waves ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Slow_Fade[] PROGMEM = "Y💡Slow_Fade ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Zoom[] PROGMEM = "Y💡Zoom ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Hot_Blob[] PROGMEM = "Y💡Hot_Blob ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Spiralus2[] PROGMEM = "Y💡Spiralus2 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Spiralus[] PROGMEM = "Y💡Spiralus ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Yves[] PROGMEM = "Y💡Yves ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Scaledemo1[] PROGMEM = "Y💡Scaledemo1 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Lava1[] PROGMEM = "Y💡Lava1 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Caleido3[] PROGMEM = "Y💡Caleido3 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Caleido2[] PROGMEM = "Y💡Caleido2 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Caleido1[] PROGMEM = "Y💡Caleido1 ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Distance_Experiment[] PROGMEM = "Y💡Distance_Experiment ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Center_Field[] PROGMEM = "Y💡Center_Field ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Waves[] PROGMEM = "Y💡Waves ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Chasing_Spirals[] PROGMEM = "Y💡Chasing_Spirals ☾@" ANIMARTRIX_UI_CONTROLS;
static const char _data_FX_mode_Rotating_Blob[] PROGMEM = "Y💡Rotating_Blob ☾@" ANIMARTRIX_UI_CONTROLS;


class ANIMartRIXMod:public ANIMartRIX {
	private:
	int hueshift = 0; // static HUE shift (16bit signed); default = neutral
	bool use_gamma = false;
	bool cycle_hue = false;
	bool boost_brightness = false;
	bool boost_contrast = false;
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
	  setSpeedFactor(speedFactor);

	  //use_gamma = SEGENV.check1;
	  use_gamma = true; // ToDO: move to usermod options
	  cycle_hue = SEGENV.check1;
	  boost_brightness = SEGENV.check2;
	  boost_contrast = SEGENV.check3;

		if (cycle_hue) {
			unsigned tt = strip.now;            // use strip time as timebase - change to millis() if you see jitter or stuttering
			hueshift = (tt << 4) | (tt & 0x0F); // default shift based on time, without speedup or slowdown => one cycle in 4 seconds
			if (SEGMENT.intensity > 128) {
				// tt = (uint64_t(tt) * (31 + SEGMENT.intensity - 127)) / 32;    // => faster up to 4x (128/32)
				hueshift = (uint64_t(tt) * (31 + SEGMENT.intensity - 127)) / 2;  // try to preserve accuracy: time/32 * 16 => time/2
			} else if (SEGMENT.intensity < 127) {
				//tt = (uint64_t(tt) * 22) / (21 + 127 - SEGMENT.intensity);     // => slower down to 1/7 (22/148)
				hueshift = (uint64_t(tt) * (22*16)) / (21 + 127 - SEGMENT.intensity); // try to preserve accuracy, by embedding "<<4" (*16) into the main multiplication
			}
		} else { // !cycle_hue
			hueshift = (128 - SEGMENT.intensity) * 256;  // static HUE shift
		}
	}

	// enhance middle ranges contrast (S-Function)
	static inline float enhanceContrast(float color) {
		if (color < 1.0f) return 0.0f; // shortcut for black
		#if defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32P4)
			// floating point - faster when FPU is present
			float x = color / 255.0f; // normalize to [0, 1]
			float y = x * x * (3.0f - 2.0f * x); // smoothstep (S-curve) for contrast
			float result = y * 255.0f; // scale back to [0, 255]
			return min(max(result, 0.0f), 255.0f); // clamp to allowed range
		#else
			// fixed point with rounding - faster when no FPU
			unsigned xcol = color;
			unsigned ycol1 = ((xcol * xcol * 3) +127);                  // first part:  y1 = 255 * (x * x * 3)
			unsigned ycol2 = ((xcol * xcol * 2 * xcol) +32512) / 255;   // second part: y2 = 255 * (x * x * 2 * x)
			if (ycol2 > ycol1) return 0.0f; // catch underflow (overflow can't happen)
			else return min(255.0f, float(ycol1 - ycol2) / 255.0f);     // clamp to allowed range
		#endif
	}

	// enhance brightness (sqrt function)
	static inline uint_fast8_t enhanceBrightness(float color) {
		// square root - provides gentle and jump-free enhancement of lower brightness pixels
		if (color < 0.125f) return 0;   // shortcut for black
		if (color > 255.0f) return 255; // shortcut for max color value
		// floating point: slow due to sqrtf()
		//   return roundf(sqrtf(color/255.0f) * 255.0f);
		// fixed point: faster
		uint32_t col32 = 0.5f + (255.0f * color); // = ( color / 255 ) * 65025 ; 16bit fixed-point representation of color/255
		return sqrt32_bw(col32);                  // => equal to sqrt((color / 255) * 255; produces [0-255] output
	}

	// gamma correction
	static inline uint32_t applyGamma24(uint32_t colIn) {
		#ifdef _MoonModules_WLED_   // upstream WLED does not need gamma-correction before setPixelColor
			uint8_t colR = gamma8(R(colIn)); 
			uint8_t colG = gamma8(G(colIn)); 
			uint8_t colB = gamma8(B(colIn));
			return RGBW32(colR, colG, colB, 0U);
		#else
			return colIn;  // do nothing
		#endif
	}

	inline uint32_t processColor(rgb pixel) const {
		if (boost_contrast) {
			// enhance contrast - keep "float" for better color accuracy
			pixel.red = enhanceContrast(pixel.red);
			pixel.green = enhanceContrast(pixel.green);
			pixel.blue = enhanceContrast(pixel.blue);
		}
		uint32_t colOut;
		if (boost_brightness) {
			// enhance brightness, convert colors from float to integer
			uint8_t colR = enhanceBrightness(pixel.red);
			uint8_t colG = enhanceBrightness(pixel.green);
			uint8_t colB = enhanceBrightness(pixel.blue);
			colOut = RGBW32(colR, colG, colB, 0U);
		} else {
			// color conversion only; +0.5f for rounding
			uint8_t colR = pixel.red+0.5f;
			uint8_t colG = pixel.green+0.5f;
			uint8_t colB = pixel.blue+0.5f;
			colOut = RGBW32(colR, colG, colB,0U);
		}

		// experimental: HUE shift
		if (cycle_hue || (abs(hueshift) > 255)) { // cycle HUE selected, or manual HUE at lest 1 left/right from center of slider
			CHSV32 cc;
			rgb2hsv(colOut, cc);
			cc.h = cc.h + unsigned(hueshift); // works due to 2's complement
			hsv2rgb(cc, colOut);
		}

		return use_gamma ? applyGamma24(colOut) : colOut;
	}

	void setPixelColor(int x, int y, rgb pixel) override {
		SEGMENT.setPixelColorXY(x, y, processColor(pixel));
	}

	void setPixelColor(int index, rgb pixel) override {
		// get x and y, so we can us setPixelColorXY() - faster in WLEDMM
		int x = index % num_x;
		int y = index / num_x;
		SEGMENT.setPixelColorXY(x,y, processColor(pixel));
  	}

	// Add any extra custom effects not part of the ANIMartRIX libary here
};
ANIMartRIXMod anim;

uint16_t mode_Module_Experiment10() {
	anim.initEffect(); 
	anim.Module_Experiment10();
	return FRAMETIME;
}
uint16_t mode_Module_Experiment9() { 
	anim.initEffect(); 
	anim.Module_Experiment9();
	return FRAMETIME;
}
uint16_t mode_Module_Experiment8() { 
	anim.initEffect(); 
	anim.Module_Experiment8();
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
uint16_t mode_Module_Experiment3() { 
	anim.initEffect(); 
	anim.Module_Experiment3();
	return FRAMETIME;
}
uint16_t mode_Module_Experiment2() { 
	anim.initEffect(); 
	anim.Module_Experiment2();
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
uint16_t mode_Complex_Kaleido_2() { 
	anim.initEffect(); 
	anim.Complex_Kaleido_2();
	return FRAMETIME;
}
uint16_t mode_Complex_Kaleido() { 
	anim.initEffect(); 
	anim.Complex_Kaleido();
	return FRAMETIME;
}
uint16_t mode_SM10() { 
	anim.initEffect(); 
	anim.SM10();
	return FRAMETIME;
}
uint16_t mode_SM9() { 
	anim.initEffect(); 
	anim.SM9();
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
uint16_t mode_SM6() { 
	anim.initEffect(); 
	anim.SM6();
	return FRAMETIME;
}
uint16_t mode_SM5() { 
	anim.initEffect(); 
	anim.SM5();
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
uint16_t mode_RGB_Blobs5() { 
	anim.initEffect(); 	
	anim.RGB_Blobs5();
	return FRAMETIME;
}
uint16_t mode_RGB_Blobs4() { 
	anim.initEffect(); 
	anim.RGB_Blobs4();
	return FRAMETIME;
}
uint16_t mode_RGB_Blobs3() { 
	anim.initEffect(); 
	anim.RGB_Blobs3();
	return FRAMETIME;
}
uint16_t mode_RGB_Blobs2() { 
	anim.initEffect(); 
	anim.RGB_Blobs2();
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

      strip.addEffect(203, &mode_Module_Experiment10, _data_FX_mode_Module_Experiment10);
      strip.addEffect(204, &mode_Module_Experiment9, _data_FX_mode_Module_Experiment9);
      strip.addEffect(205, &mode_Module_Experiment8, _data_FX_mode_Module_Experiment8);
      strip.addEffect(206, &mode_Module_Experiment7, _data_FX_mode_Module_Experiment7);
      strip.addEffect(207, &mode_Module_Experiment6, _data_FX_mode_Module_Experiment6);
      strip.addEffect(208, &mode_Module_Experiment5, _data_FX_mode_Module_Experiment5);
      strip.addEffect(209, &mode_Module_Experiment4, _data_FX_mode_Module_Experiment4);
      strip.addEffect(210, &mode_Zoom2, _data_FX_mode_Zoom2);
      strip.addEffect(211, &mode_Module_Experiment3, _data_FX_mode_Module_Experiment3);
      strip.addEffect(212, &mode_Module_Experiment2, _data_FX_mode_Module_Experiment2);
      strip.addEffect(213, &mode_Module_Experiment1, _data_FX_mode_Module_Experiment1);
      strip.addEffect(214, &mode_Parametric_Water, _data_FX_mode_Parametric_Water);
      strip.addEffect(215, &mode_Water, _data_FX_mode_Water);
      strip.addEffect(216, &mode_Complex_Kaleido_6, _data_FX_mode_Complex_Kaleido_6);
      strip.addEffect(217, &mode_Complex_Kaleido_5, _data_FX_mode_Complex_Kaleido_5);
      strip.addEffect(218, &mode_Complex_Kaleido_4, _data_FX_mode_Complex_Kaleido_4);
      strip.addEffect(219, &mode_Complex_Kaleido_3, _data_FX_mode_Complex_Kaleido_3);
      strip.addEffect(220, &mode_Complex_Kaleido_2, _data_FX_mode_Complex_Kaleido_2);
      strip.addEffect(221, &mode_Complex_Kaleido, _data_FX_mode_Complex_Kaleido);
      strip.addEffect(222, &mode_SM10, _data_FX_mode_SM10);
      strip.addEffect(223, &mode_SM9, _data_FX_mode_SM9);
      strip.addEffect(224, &mode_SM8, _data_FX_mode_SM8);
      // strip.addEffect(225, &mode_SM7, _data_FX_mode_SM7);
      strip.addEffect(226, &mode_SM6, _data_FX_mode_SM6);
      strip.addEffect(227, &mode_SM5, _data_FX_mode_SM5);
      strip.addEffect(228, &mode_SM4, _data_FX_mode_SM4);
      strip.addEffect(229, &mode_SM3, _data_FX_mode_SM3);
      strip.addEffect(230, &mode_SM2, _data_FX_mode_SM2);
      strip.addEffect(231, &mode_SM1, _data_FX_mode_SM1);
      strip.addEffect(232, &mode_Big_Caleido, _data_FX_mode_Big_Caleido);
      strip.addEffect(233, &mode_RGB_Blobs5, _data_FX_mode_RGB_Blobs5);
      strip.addEffect(234, &mode_RGB_Blobs4, _data_FX_mode_RGB_Blobs4);
      strip.addEffect(235, &mode_RGB_Blobs3, _data_FX_mode_RGB_Blobs3);
      strip.addEffect(236, &mode_RGB_Blobs2, _data_FX_mode_RGB_Blobs2);
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
