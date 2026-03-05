#pragma once

#include "wled.h"
#include "colors.h" // include CHSV32 class by @dedehai
#include "fcn_declare.h" // utilities

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

#define ANIMARTRIX_UI_MONITOR "Speed(nothing),HUE Change(nothing),Audio Strength,Audio Decay,Audio Detector ID,Amplify,Amplify,;!,!;!;1f;c1=255,c2=32,c3=0,o1=0,o2=0"
static const char _data_FX_mode_AudioMon1D[] PROGMEM = "Y💡AudioMonitor 1D ☾@" ANIMARTRIX_UI_MONITOR;

#define ANIMARTRIX_UI_CONTROLS "Speed,HUE Change,Audio Strength,Audio Decay,,cycle HUE,boost Brightness,boost Contrast;;1;2f;c1=0,c2=32,o2=0"

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

// local utility functions
//

// Attach to audiosource, or fall back to simulateSound
static um_data_t* getAudioDataOrSim() {
  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  return um_data;
}

// better "map" that can be used when in_min = out_min = 0. 
static inline uint32_t map0(uint32_t val, uint32_t in_max, uint32_t out_max) {
		// Fast and accurate (error always below 0.5)
   if (in_max == 0) return 0; // avoid division by zero
   return ( (val*out_max) + (in_max/2) ) / in_max;     // +(in_max/2) for rounding
}
#if 0 // not used yet
// a variant of map0() that handles output ranges not starting at 0 - but still requires val in [0 ... in_max]
static inline int32_t map0(uint32_t val, uint32_t in_max, int32_t out_min, int32_t out_max) {
  if (out_min > out_max) std::swap(out_min, out_max); // a hack, just to treat inverted ranges without producing overflows
  int range = out_max - out_min;
  return int(map0(val, in_max, unsigned(range))) + out_min;
}
#endif

// global settings - shared between ANIMartRIXMod and AnimartrixUsermod
//
static uint8_t animartrix_use_gamma = 1; // default = enabled. Can be disabled to get the "legacy" gamma-free look
#ifdef USERMOD_AUDIOREACTIVE
static uint8_t animartrix_detectorID = 4; // default = bass detector
#else
static uint8_t animartrix_detectorID = 0; // not AR usermod -> default = no audio
#endif

// ANIMartRIXMod class
//

class ANIMartRIXMod:public ANIMartRIX {
	private:
	int hueshift = 0; // static HUE shift (16bit signed); default = neutral
	bool use_gamma = false;
	bool cycle_hue = false;
	bool boost_brightness = false;
	bool boost_contrast = false;

  // --- audio core ------------------------------------------------------------------------------

		// ToDo 1: sliders
		// * for HUE change amount by audio (handling based on cycle_hue: static shift, or speedup/slowdown)
		// * for AUDIO filtering: instant -> 1s decay -> what else? Maybe accumulate changes but use speed limit?

		// TODO 2
		// attach to audiosource
		// filter raw audio input
		// adjust HUE shift based on audio data
		// (maybe) allow to adjust brightness
		// (details) compare to audioreactive palettes by @netmindz

		// ToDo 3: user option to configure audio input
		// none, peak detection, zcr(major frequency), pressure, volumeSmth, High freqs (fftbin[7-10]), mid freqs (fftbin[4-8]), low freqs (fftbin[0-4])

		// detection engine
		#define NUM_DETECTORS 7 // total, including "none"
		#define DET_NONE     0     // no audio
		#define DET_VOLUME   1     // AR volumeSmth (relative)
		#define DET_PRESSURE 2     // AR soundPressure (absolute)
		#define DET_ZCR      3     // AR ZeroCrossingCount (music "density")
		#define DET_BASS     4     // AR bass (FFT channel 1+2)
		#define DET_MID      5     // AR voices & melody (FFT channel 5,7,8,10)
		#define DET_HIGH     6     // AR high frequencies - high-hats, pipes, high pitch (FFT channel 12,13,15)

		// get audio - either soundSim or audioReactive UM
		static float getAudio(unsigned detectorID) {
			constexpr float M_LOG_256 = 5.54517744f;    // log(256) = max value
			constexpr float M_LOG_3   = 1.098612289f;   // log(3)   = minimal value
  		um_data_t *um_data   = getAudioDataOrSim();
  		float   volumeSmth   = *(float*)    um_data->u_data[0];
  		// int16_t volumeRaw = *(int16_t*)  um_data->u_data[1];
  		uint8_t *fftResult   = (uint8_t*)   um_data->u_data[2];
  		float soundPressure  = *(float*)    um_data->u_data[9];
  		//float agcSensitivity=*(float*)    um_data->u_data[10];
  		uint16_t zCr         = *(uint16_t*) um_data->u_data[11];

			bool isSilence = volumeSmth < 1.0f;
			if (isSilence) return 0.0f;

			// extract audio based on selected detection method
			float audioSample = 0.0f;
			switch (detectorID) {
				case DET_VOLUME:     audioSample = volumeSmth;    break;
				case DET_PRESSURE:   audioSample = soundPressure; break;
				case DET_ZCR:
  				// zero crossings count
  				audioSample = float(zCr);
  				if (audioSample > 3.0f) 
						audioSample = 255.0f * (logf(audioSample) - M_LOG_3) / M_LOG_256; // log scaling - amplifies lower frequencies, reduces noise
  				else audioSample = 0.0f;
					break;
				case DET_BASS:
  				// low freqs RMS
  				audioSample = 0.6f * sqrtf(float(fftResult[1])*float(fftResult[1])    //   86 - 129 hz
                                   + float(fftResult[2])*float(fftResult[2]));  //  129 - 216 hz
					break;
				case DET_MID:
					// mid freqs RMS with some boost (voices, chorus, some instruments)
  				audioSample = sqrtf(float(fftResult[5])*float(fftResult[5])           //  430 -  560 hz
                            + float(fftResult[7])*float(fftResult[7])           //  818 - 1120 hz
                            + float(fftResult[8])*float(fftResult[8])           // 1120 - 1421 hz
                            + float(fftResult[10])*float(fftResult[10])) /2.5f; // 1895 - 2412 hz
					break;
				case DET_HIGH:
				  // high freqs RMS (hats, pipes, high rattle stuff)
  				audioSample = sqrtf(float(fftResult[12])*float(fftResult[12])          // 3015 - 3704 hz 
                            + float(fftResult[13])*float(fftResult[13])          // 3704 - 4479 hz
                            + float(fftResult[15])*float(fftResult[15])) / 2.2f; // 7106 - 9259 hz
					break;
				case DET_NONE: 
					// falls through
				default: break;
			}
			return (audioSample > 0.8f) ? audioSample : 0.0f;  // clamp silence and underflows
		}


		// filter audio
		float lastAudioData[NUM_DETECTORS] = {0.0f};
		unsigned lastAudioTime[NUM_DETECTORS] = {0};

		float processAudio(unsigned detectorID, unsigned decayMS) {
			constexpr float audioSmooth = 0.88f; // audio smoothing factor - to avoid instant flashes and very hard jumps
			if (detectorID >= NUM_DETECTORS) detectorID = 0;
			if (detectorID == 0) return 0.0f;

			unsigned timestamp = millis();
    	long delta_time = timestamp - lastAudioTime[detectorID];
    	delta_time = min(max(delta_time , 1L), 1000L);           // clamp to meaningful values
			if (delta_time < 3) return min(max(0.0f, lastAudioData[detectorID]), 255.0f); // too early, value has not changed since last time

			float newAudio = getAudio(detectorID);
			float deltaSample = newAudio - lastAudioData[detectorID]; // positive attack, negative during decay

			if ((deltaSample > 0.0f) || (decayMS < 1)) {
				// fast attack/decay with minimal filtering
				// lastAudioData[detectorID] += 0.9f * audioSmooth * deltaSample;  // experimental - 0.9 for damping of jumps
				lastAudioData[detectorID] += audioSmooth * deltaSample;
			} else {
				// slow decay: time-based linear decay; similar to AR limitSampleDynamics() function
				constexpr float bigChange = 184; // a large, expected sample value that decays to 0 in decayMS millis
        float maxDecay = - bigChange * float(delta_time) / float(decayMS); // allowed decay for elapsed time (must be negative!)
        if (deltaSample < maxDecay) deltaSample = maxDecay; // limit delta if new value is too low
				lastAudioData[detectorID] += audioSmooth * deltaSample;
			}

			lastAudioTime[detectorID] = timestamp;
			return min(max(0.0f, lastAudioData[detectorID]), 255.0f);  // clamp result to 0..255, but keep exact value internally
		}

  // --- audio core end --------------------------------------------------------------------------

	public:
		// audio processing, called once per frame per segment
		void handleAudioHUE(unsigned detectorID) {
			unsigned audioDecay = map0(SEGMENT.custom2, 255, 3500);              // 0 (instant) up to 4 seconds
			float audioShift = 255.0f * processAudio(detectorID, audioDecay);    // 0.0 ... 65025 = full "HUE turn"
			float audioStrength = float(SEGMENT.custom1) / 255.0f;               // multiplier [0...1]
			audioShift = audioShift * audioStrength;
			if (SEGMENT.custom1 > 1) hueshift = hueshift + unsigned(audioShift);
		}

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

	  use_gamma = animartrix_use_gamma > 0;  // from global usermod options
	  cycle_hue = SEGENV.check1;             // from segment checkboxes
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

		handleAudioHUE(animartrix_detectorID);
	}

	// AUDIO DEBUG: simplified variant of initEffect()
	inline int getAudioHUE() const { return hueshift; }
	void initMonitor() {
		#if !defined(WLEDMM_NO_GAMMA)
			use_gamma = animartrix_use_gamma > 0;  // from global usermod options
		#else
			use_gamma = false; // ToDO: move to usermod options
		#endif
		cycle_hue = false;
		boost_brightness = false;
		boost_contrast = false;
		hueshift = 0;
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
		if (cycle_hue || (abs(hueshift) > 255)) { // cycle HUE selected, or manual HUE if at least 1 left/right from center of slider
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

ANIMartRIXMod animAudioMon; // second object instance fir monitor, to avoid overlaping of audio processing
uint16_t mode_AudioMon() {
	// debug 1D audio monitor (gravimeter syle)
	animAudioMon.initMonitor();
	SEGMENT.custom3 = min(SEGMENT.custom3, uint8_t(NUM_DETECTORS-1));
	unsigned detectorID = SEGMENT.custom3 == 0 ? animartrix_detectorID : SEGMENT.custom3;

	animAudioMon.handleAudioHUE(detectorID);
	float volumeSmth = float(uint16_t(animAudioMon.getAudioHUE())) / 255.0f;

	if (SEGMENT.check1) volumeSmth *= 1.5f;  // amplify by 1.5 for better visibility
	if (SEGMENT.check2) volumeSmth *= 1.5f;  // amplify again by 1.5
	if (SEGENV.call == 0) {
		SEGMENT.fill(BLACK);
	}
	SEGMENT.fade_out(253);

	float mySampleAvg = volumeSmth/255.0f * float(SEGLEN-1); // map to pixels available in current segment
	unsigned segmentSampleAvg = volumeSmth / 4;
	int tempsamp = constrain(mySampleAvg,0,SEGLEN-1);       // Keep the sample from overflowing.
	for (int i=0; i<tempsamp; i++) {
		uint8_t palIndex = perlin8(i*segmentSampleAvg+millis(), 5000+i*segmentSampleAvg);
		SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(palIndex, false, true, 0));
	}
	return FRAMETIME;
}

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
		
      if (initDone || !enabled) return; // WLEDMM don't register effects twice!

      strip.addEffect(255, &mode_AudioMon, _data_FX_mode_AudioMon1D); // audio debug

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
      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

      JsonArray infoArr = user.createNestedArray(FPSTR(_name));

      String uiDomString = F("Animartrix requires the Creative Commons Attribution License CC BY-NC 3.0");
      infoArr.add(uiDomString);
	}

		// add JSON entries that go to cfg.json
		void addToConfig(JsonObject& obj) override {
		  JsonObject top = obj.createNestedObject(FPSTR(_name));                 // WLEDMM: set enabled and _name
		  top[FPSTR("enabled")] = enabled;
			top[FPSTR("gamma_correction")] = animartrix_use_gamma;
			animartrix_detectorID = constrain(animartrix_detectorID, 0, NUM_DETECTORS);
			top[FPSTR("audio_detector")] = animartrix_detectorID;
		}

		bool readFromConfig(JsonObject& root) override {
			JsonObject top = root[FPSTR(_name)]; //WLEDMM
		  bool configComplete = !top.isNull();
		  // remember previous values
		  auto oldEnabled = enabled;
			// read config
		  configComplete &= getJsonValue(top[FPSTR("enabled")], enabled);
		 	configComplete &= getJsonValue(top[FPSTR("gamma_correction")], animartrix_use_gamma);
			configComplete &= getJsonValue(top[FPSTR("audio_detector")], animartrix_detectorID);
			animartrix_detectorID = constrain(animartrix_detectorID, 0, NUM_DETECTORS);
			if (oldEnabled != enabled) setup();  // re-run setup if enabled status changed
		  return configComplete;
		}

		/*
		 * appendConfigData() is called when user enters usermod settings page
		 * it may add additional metadata for certain entry fields (adding drop down is possible)
		 */
		void appendConfigData() override {
			// create dropdown for "gamma_correction"
		  oappend(SET_F("dd=addDropdown('")); oappend(String(FPSTR(_name)).c_str()); oappend(SET_F("','gamma_correction');"));
		  oappend(SET_F("addOption(dd,'On  (⎌)',1);"));
		  oappend(SET_F("addOption(dd,'Off',0);"));

		  oappend(SET_F("dd1=addDropdown('")); oappend(String(FPSTR(_name)).c_str()); oappend(SET_F("','audio_detector');"));
		  oappend(SET_F("addOption(dd1,'Sound Level (relative)',1);"));
		  oappend(SET_F("addOption(dd1,'Sound Pressure (absolute)',2);"));
		  oappend(SET_F("addOption(dd1,'ZeroCrossings (density)',3);"));
		  oappend(SET_F("addOption(dd1,'Bass Frequencies (⎌)',4);"));
		  oappend(SET_F("addOption(dd1,'Mid Frequencies (voices)',5);"));
		  oappend(SET_F("addOption(dd1,'Very High Frequencies',6);"));
		  oappend(SET_F("addOption(dd1,'No Audio',0);"));
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
