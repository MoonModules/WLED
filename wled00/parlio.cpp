#include "wled.h"
#ifdef SOC_PARLIO_SUPPORTED
#include "driver/parlio_tx.h"
#include "portmacro.h"

// --- Namespace for specialized, high-performance worker functions ---
namespace LedMatrixDetail {

// This intermediate step is common to all packing functions.
// It transposes the data for 32 time-slices into a cache-friendly temporary buffer.
inline void transpose_32_slices(
    uint32_t (&transposed_slices)[32], // Output buffer (on stack)
    const uint8_t* input_buffer,
    const uint32_t pixel_in_pin,
    const uint32_t component_in_pixel,
    const uint32_t pixels_per_pin,
    const uint32_t num_active_pins,
    const uint32_t COMPONENTS_PER_PIXEL,
    const uint32_t* waveform_cache,
    const uint8_t* brightness_cache) 
{
    memset(transposed_slices, 0, sizeof(uint32_t) * 32);

    for (uint32_t pin = 0; pin < num_active_pins; ++pin) {
        const uint32_t pixel_idx = (pin * pixels_per_pin) + pixel_in_pin;
        const uint32_t component_idx = (pixel_idx * COMPONENTS_PER_PIXEL) + component_in_pixel;
        const uint8_t data_byte = brightness_cache[input_buffer[component_idx]];
        const uint32_t waveform = waveform_cache[data_byte];
        const uint32_t pin_bit = (1 << pin);

        uint8_t b;

        b = waveform & 0xFF;
        if ((b >> 7) & 1) transposed_slices[0]  |= pin_bit; if ((b >> 6) & 1) transposed_slices[1]  |= pin_bit;
        if ((b >> 5) & 1) transposed_slices[2]  |= pin_bit; if ((b >> 4) & 1) transposed_slices[3]  |= pin_bit;
        if ((b >> 3) & 1) transposed_slices[4]  |= pin_bit; if ((b >> 2) & 1) transposed_slices[5]  |= pin_bit;
        if ((b >> 1) & 1) transposed_slices[6]  |= pin_bit; if ((b >> 0) & 1) transposed_slices[7]  |= pin_bit;

        b = (waveform >> 8) & 0xFF;
        if ((b >> 7) & 1) transposed_slices[8]  |= pin_bit; if ((b >> 6) & 1) transposed_slices[9]  |= pin_bit;
        if ((b >> 5) & 1) transposed_slices[10] |= pin_bit; if ((b >> 4) & 1) transposed_slices[11] |= pin_bit;
        if ((b >> 3) & 1) transposed_slices[12] |= pin_bit; if ((b >> 2) & 1) transposed_slices[13] |= pin_bit;
        if ((b >> 1) & 1) transposed_slices[14] |= pin_bit; if ((b >> 0) & 1) transposed_slices[15] |= pin_bit;

        b = (waveform >> 16) & 0xFF;
        if ((b >> 7) & 1) transposed_slices[16] |= pin_bit; if ((b >> 6) & 1) transposed_slices[17] |= pin_bit;
        if ((b >> 5) & 1) transposed_slices[18] |= pin_bit; if ((b >> 4) & 1) transposed_slices[19] |= pin_bit;
        if ((b >> 3) & 1) transposed_slices[20] |= pin_bit; if ((b >> 2) & 1) transposed_slices[21] |= pin_bit;
        if ((b >> 1) & 1) transposed_slices[22] |= pin_bit; if ((b >> 0) & 1) transposed_slices[23] |= pin_bit;

        b = (waveform >> 24) & 0xFF;
        if ((b >> 7) & 1) transposed_slices[24] |= pin_bit; if ((b >> 6) & 1) transposed_slices[25] |= pin_bit;
        if ((b >> 5) & 1) transposed_slices[26] |= pin_bit; if ((b >> 4) & 1) transposed_slices[27] |= pin_bit;
        if ((b >> 3) & 1) transposed_slices[28] |= pin_bit; if ((b >> 2) & 1) transposed_slices[29] |= pin_bit;
        if ((b >> 1) & 1) transposed_slices[30] |= pin_bit; if ((b >> 0) & 1) transposed_slices[31] |= pin_bit;
    }
}

void __attribute__((hot)) process_1bit(uint8_t* buffer, const uint32_t* transposed_slices) {
    uint32_t packed_word = 0;
    for (int i = 0; i < 32; ++i) {
        if (transposed_slices[i]) {
            packed_word |= (1 << i);
        }
    }
    reinterpret_cast<uint32_t*>(buffer)[0] = packed_word;
}

void __attribute__((hot)) process_2bit(uint8_t* buffer, const uint32_t* transposed_slices) {
    uint32_t* out = reinterpret_cast<uint32_t*>(buffer);
    uint32_t word0 = 0, word1 = 0;
    for(int i = 0; i < 16; ++i) word0 |= (transposed_slices[i] << (i * 2));
    for(int i = 0; i < 16; ++i) word1 |= (transposed_slices[i+16] << (i * 2));
    out[0] = word0;
    out[1] = word1;
}

void __attribute__((hot)) process_4bit(uint8_t* buffer, const uint32_t* transposed_slices) {
    uint32_t* out = reinterpret_cast<uint32_t*>(buffer);
    uint32_t word0=0, word1=0, word2=0, word3=0;
    for(int i = 0; i < 8; ++i) word0 |= (transposed_slices[i] << (i * 4));
    for(int i = 0; i < 8; ++i) word1 |= (transposed_slices[i+8] << (i * 4));
    for(int i = 0; i < 8; ++i) word2 |= (transposed_slices[i+16] << (i * 4));
    for(int i = 0; i < 8; ++i) word3 |= (transposed_slices[i+24] << (i * 4));
    out[0] = word0; out[1] = word1; out[2] = word2; out[3] = word3;
}

void __attribute__((hot)) process_8bit(uint8_t* buffer, const uint32_t* transposed_slices) {
    // Cast the output buffer to write 32-bit words at a time.
    uint32_t* out = reinterpret_cast<uint32_t*>(buffer);

    // We have 32 bytes to write, so we do it in 8 chunks of 4 bytes (uint32_t).
    for (int i = 0; i < 8; ++i) {
        const int base_idx = i * 4;
        // Manually pack four 8-bit values into one 32-bit word.
        // The values from transposed_slices are implicitly truncated to bytes.
        uint32_t packed_word = (transposed_slices[base_idx + 0]) |
                               (transposed_slices[base_idx + 1] << 8) |
                               (transposed_slices[base_idx + 2] << 16) |
                               (transposed_slices[base_idx + 3] << 24);
        // Perform a single, efficient 32-bit write.
        out[i] = packed_word;
    }
}

void __attribute__((hot)) process_16bit(uint16_t* buffer, const uint32_t* transposed_slices) {
    // Cast the output buffer to write 32-bit words at a time.
    uint32_t* out = reinterpret_cast<uint32_t*>(buffer);

    // We have 64 bytes to write, so we do it in 16 chunks of 4 bytes (uint32_t).
    for (int i = 0; i < 16; ++i) {
        const int base_idx = i * 2;
        // Manually pack two 16-bit values into one 32-bit word.
        uint32_t packed_word = (transposed_slices[base_idx + 0]) |
                               (transposed_slices[base_idx + 1] << 16);
        // Perform a single, efficient 32-bit write.
        out[i] = packed_word;
    }
}

} // namespace detail

// 1. Add the color_order parameter to the function signature
void create_transposed_led_output_optimized(
  const uint8_t* input_buffer,
  uint16_t* output_buffer,
  const uint32_t pixels_per_pin,
  const uint32_t num_active_pins,
  const bool is_rgbw,
  const uint8_t bri,
  const uint8_t color_order) 
{
  static uint32_t waveform_cache[256];
  static uint8_t brightness_cache[256];
  static uint8_t last_bri = 0;

  static const uint16_t bitpatterns[16] = {
      0b1000100010001000, 0b1000100010001110, 0b1000100011101000, 0b1000100011101110,
      0b1000111010001000, 0b1000111010001110, 0b1000111011101000, 0b1000111011101110,
      0b1110100010001000, 0b1110100010001110, 0b1110100011101000, 0b1110100011101110,
      0b1110111010001000, 0b1110111010001110, 0b1110111011101000, 0b1110111011101110,
  };

  if (bri != last_bri) {
    for (int i = 0; i < 256; ++i) {
      brightness_cache[i] = (gamma8(i) * bri) >> 8;
    }
    for (int i = 0; i < 256; ++i) {
      const uint16_t p1 = bitpatterns[i >> 4];
      const uint16_t p2 = bitpatterns[i & 0x0F];
      waveform_cache[i] = (uint32_t(p2) << 16) | p1;
    }
    last_bri = bri;
  }

  const uint32_t COMPONENTS_PER_PIXEL = is_rgbw ? 4 : 3;
  const uint32_t WAVEFORM_WORDS_PER_PIXEL = COMPONENTS_PER_PIXEL * 32;
  const uint32_t total_output_words = pixels_per_pin * WAVEFORM_WORDS_PER_PIXEL;

  if (total_output_words == 0) return;

  uint8_t bit_width;
  if (num_active_pins <= 1) bit_width = 1;
  else if (num_active_pins <= 2) bit_width = 2;
  else if (num_active_pins <= 4) bit_width = 4;
  else if (num_active_pins <= 8) bit_width = 8;
  else bit_width = 16;

  const size_t total_bytes = (total_output_words * bit_width + 7) / 8;
  memset(output_buffer, 0, total_bytes);

  uint8_t* out_base_ptr = reinterpret_cast<uint8_t*>(output_buffer);

  uint8_t component_map[4] = { 0, 1, 2, 3 }; // Default to RGB(W)
  switch (color_order) {
    case COL_ORDER_GRB: component_map[0] = 1; component_map[1] = 0; component_map[2] = 2; break; // G, R, B
    case COL_ORDER_RGB: break; // Default is already RGB
    case COL_ORDER_BRG: component_map[0] = 2; component_map[1] = 0; component_map[2] = 1; break; // B, R, G
    case COL_ORDER_RBG: component_map[0] = 0; component_map[1] = 2; component_map[2] = 1; break; // R, B, G
    case COL_ORDER_BGR: component_map[0] = 2; component_map[1] = 1; component_map[2] = 0; break; // B, G, R
    case COL_ORDER_GBR: component_map[0] = 1; component_map[1] = 2; component_map[2] = 0; break; // G, B, R
  }
  // The W component (if it exists) is always the last one.
  if (is_rgbw) component_map[3] = 3;

  // --- Main Processing Loop ---
  for (uint32_t pixel_in_pin = 0; pixel_in_pin < pixels_per_pin; ++pixel_in_pin) {

    for (uint32_t component_in_pixel = 0; component_in_pixel < COMPONENTS_PER_PIXEL; ++component_in_pixel) {

      const uint32_t input_component = component_map[component_in_pixel];

      uint32_t transposed_slices[32];

      LedMatrixDetail::transpose_32_slices(transposed_slices, input_buffer, pixel_in_pin,
        input_component, pixels_per_pin, num_active_pins,
        COMPONENTS_PER_PIXEL, waveform_cache, brightness_cache);

      const uint32_t component_start_word = (pixel_in_pin * WAVEFORM_WORDS_PER_PIXEL) + (component_in_pixel * 32);
      uint8_t* current_out_ptr = out_base_ptr + (component_start_word * bit_width / 8);

      switch (bit_width) {
      case 1: LedMatrixDetail::process_1bit(current_out_ptr, transposed_slices); break;
      case 2: LedMatrixDetail::process_2bit(current_out_ptr, transposed_slices); break;
      case 4: LedMatrixDetail::process_4bit(current_out_ptr, transposed_slices); break;
      case 8: LedMatrixDetail::process_8bit(current_out_ptr, transposed_slices); break;
      case 16: LedMatrixDetail::process_16bit(reinterpret_cast<uint16_t*>(current_out_ptr), transposed_slices); break;
      }
    }
  }
}

parlio_tx_unit_handle_t parlio_tx_unit = NULL;
parlio_tx_unit_config_t parlio_config = parlio_tx_unit_config_t();
parlio_transmit_config_t transmit_config = {
    .idle_value = 0x00, // the idle value will force the OE line to low, thus enable the output
    .flags = {
        .queue_nonblocking = 1,
        .loop_transmission = 0,
    }
};

static portMUX_TYPE parlio_spinlock = portMUX_INITIALIZER_UNLOCKED;

uint8_t IRAM_ATTR __attribute__((hot)) show_parlio(uint8_t* parallelPins, uint32_t length, uint8_t* buffer_in, uint8_t bri, bool isRGBW, uint8_t outputs, uint16_t leds_per_output, uint8_t color_order) {

  if (length != outputs * leds_per_output) {
    delay(100);
    USER_PRINTLN("Parallel IO isn't set correctly. Check length, outputs, and LEDs per output.");
    return 1;
  }

  #ifdef PARLIO_TIMER
  unsigned long timer = micros();
  #endif

  static bool parlio_setup_done = false;
  static int last_outputs = -1;
  static int last_leds_per_output = -1;
  
  outputs = outputs > SOC_PARLIO_TX_UNIT_MAX_DATA_WIDTH ? SOC_PARLIO_TX_UNIT_MAX_DATA_WIDTH : outputs;

  if (!parlio_setup_done || outputs != last_outputs || leds_per_output != last_leds_per_output) {

    parlio_config.clk_src = PARLIO_CLK_SRC_DEFAULT;
    if (outputs <= 1)       parlio_config.data_width =  1;
    else if (outputs <= 2)  parlio_config.data_width =  2;
    else if (outputs <= 4)  parlio_config.data_width =  4;
    else if (outputs <= 8)  parlio_config.data_width =  8;
    else                    parlio_config.data_width = 16;
    parlio_config.clk_in_gpio_num = gpio_num_t(-1);
    parlio_config.valid_gpio_num = gpio_num_t(-1);
    parlio_config.clk_out_gpio_num = gpio_num_t(-1);
    for (int i = 0; i < SOC_PARLIO_TX_UNIT_MAX_DATA_WIDTH; ++i) {
      parlio_config.data_gpio_nums[i] = gpio_num_t(parallelPins[i]);
    }
    #ifdef PARLIO_AUTO_OVERCLOCK // This has caused minor annoying glitching.
    if (leds_per_output <= 256) {
        parlio_config.output_clk_freq_hz = 1200000 * 4;
    } else if (leds_per_output <= 512) {
        parlio_config.output_clk_freq_hz = 1100000 * 4;
    } else {
        parlio_config.output_clk_freq_hz = 800000 * 4;
    }
    #else
    parlio_config.output_clk_freq_hz = 800000 * 4;
    #endif
    parlio_config.valid_start_delay = 0; // 16-bit max any number >0 seems to fail. 
    parlio_config.valid_stop_delay = 0; // 16-bit max but any number >0 seems to fail.
    parlio_config.dma_burst_size = 64; // may not exceed 64 on PSRAM and must be power of 2 (1,2,4,8,16,32,64) and <=4 fails.
    parlio_config.trans_queue_depth = 16;
    parlio_config.max_transfer_size = 65535;
    parlio_config.flags.clk_gate_en = 0;
    parlio_config.flags.io_loop_back = 0;
    parlio_config.flags.allow_pd = 0;
    parlio_config.flags.invert_valid_out = 0;

    if(parlio_tx_unit != NULL) {
      ESP_ERROR_CHECK(parlio_tx_unit_wait_all_done(parlio_tx_unit, -1));
      ESP_ERROR_CHECK(parlio_tx_unit_disable(parlio_tx_unit));
      ESP_ERROR_CHECK(parlio_del_tx_unit(parlio_tx_unit));
      parlio_tx_unit = NULL;
    }

    ESP_ERROR_CHECK(parlio_new_tx_unit(&parlio_config, &parlio_tx_unit));
    ESP_ERROR_CHECK(parlio_tx_unit_enable(parlio_tx_unit));
    last_outputs = outputs;
    last_leds_per_output = leds_per_output;
    parlio_setup_done = true;
    USER_PRINTF("Parallel IO configured for %u bit width and clock speed %u KHz and %u outputs.\n",parlio_config.data_width, parlio_config.output_clk_freq_hz/1000/4, outputs);
    for (uint8_t i = 0; i < SOC_PARLIO_TX_UNIT_MAX_DATA_WIDTH; i++) {
      const char* status = "";
      if (i >= parlio_config.data_width || (i >= outputs && i < parlio_config.data_width)) {
        status = "[ignored]";
      } else if (i <= outputs) {
        if (parlio_config.data_gpio_nums[i] == -1) status = "[missing]";
      }
      DEBUG_PRINTF("Parallel IO Output %u = GPIO %d %s\n",
                  (unsigned int)(i + 1),
                  parlio_config.data_gpio_nums[i],
                  status);
    }
    return 0; // let's give it a frame to set up.
  }

  static byte* parallel_buffer_remapped = NULL;
#ifdef WLEDMM_REMAP_AT_OUTPUT
  static byte* parallel_buffer_remapped1 = (byte*)heap_caps_calloc_prefer((1024 * 16 * 4) + 15, sizeof(byte), 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA, MALLOC_CAP_DMA);
  static byte* parallel_buffer_remapped2 = (byte*)heap_caps_calloc_prefer((1024 * 16 * 4) + 15, sizeof(byte), 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA, MALLOC_CAP_DMA);
#endif 
  static uint16_t* parallel_buffer_repacked = NULL;
  static uint16_t* parallel_buffer_repacked1 = (uint16_t*)heap_caps_calloc_prefer((1024 * 16 * 16), 1, 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA | MALLOC_CAP_CACHE_ALIGNED, MALLOC_CAP_DMA);
  static uint16_t* parallel_buffer_repacked2 = (uint16_t*)heap_caps_calloc_prefer((1024 * 16 * 16), 1, 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA | MALLOC_CAP_CACHE_ALIGNED, MALLOC_CAP_DMA);

  if (parallel_buffer_repacked == NULL) parallel_buffer_repacked = parallel_buffer_repacked1;
#ifdef WLEDMM_REMAP_AT_OUTPUT
  if (parallel_buffer_remapped == NULL) parallel_buffer_remapped = parallel_buffer_remapped1;

  uint32_t* mappingTable = strip.getCustomMappingTable();
  uint32_t mappingTableSize = strip.getCustomMappingTableSize();
  uint8_t my_bytes_per_pixel = isRGBW ? 4 : 3;

  uint32_t buf_pos = 0;

  for (uint32_t i = 0; i < length; i++) {
    uint32_t dest_pixel_index = i;
    if (mappingTable && i < mappingTableSize) {
      dest_pixel_index = mappingTable[i];
    }
    uint32_t dest_byte_pos = dest_pixel_index * my_bytes_per_pixel;
    parallel_buffer_remapped[dest_byte_pos + 0] = buffer_in[buf_pos++];
    parallel_buffer_remapped[dest_byte_pos + 1] = buffer_in[buf_pos++];
    parallel_buffer_remapped[dest_byte_pos + 2] = buffer_in[buf_pos++];
    if (isRGBW) {
      parallel_buffer_remapped[dest_byte_pos + 3] = buffer_in[buf_pos++];
    }
  }
#else
  parallel_buffer_remapped = buffer_in;
  color_order = COL_ORDER_RGB; // This isn't actually changing the color order - we're already there from the BusNetwork doing the right thing pixel-by-pixel.
#endif

  create_transposed_led_output_optimized(parallel_buffer_remapped, parallel_buffer_repacked, leds_per_output, outputs, isRGBW, bri, color_order);

  // Calculate the exact size of ONE PIXEL's data in bits and bytes.
  const uint32_t symbols_per_pixel = isRGBW ? 128 : 96;
  const uint32_t bits_per_pixel = symbols_per_pixel * parlio_config.data_width;
  const uint32_t bytes_per_pixel = (bits_per_pixel + 7) / 8;
  const uint32_t HW_MAX_BYTES_PER_CHUNK = parlio_config.max_transfer_size;
  const uint16_t max_leds_per_chunk = (bytes_per_pixel > 0) ? (HW_MAX_BYTES_PER_CHUNK / bytes_per_pixel) : 0;
  const uint8_t num_chunks = (leds_per_output + max_leds_per_chunk - 1) / max_leds_per_chunk;

  uint32_t chunk_bits[4];
  const uint8_t* chunk_ptrs[4];
  uint32_t leds_remaining = leds_per_output;
  const size_t chunk_stride_bytes = (size_t)max_leds_per_chunk * bytes_per_pixel;

  // Chunk 1
  uint32_t leds_in_chunk = (leds_remaining < max_leds_per_chunk) ? leds_remaining : max_leds_per_chunk;
  chunk_bits[0] = leds_in_chunk * bits_per_pixel;
  chunk_ptrs[0] = (const uint8_t*)parallel_buffer_repacked;
  leds_remaining -= leds_in_chunk;

  // Chunk 2
  leds_in_chunk = (leds_remaining < max_leds_per_chunk) ? leds_remaining : max_leds_per_chunk;
  chunk_bits[1] = leds_in_chunk * bits_per_pixel;
  chunk_ptrs[1] = chunk_ptrs[0] + chunk_stride_bytes;
  leds_remaining -= leds_in_chunk;

  // Chunk 3
  leds_in_chunk = (leds_remaining < max_leds_per_chunk) ? leds_remaining : max_leds_per_chunk;
  chunk_bits[2] = leds_in_chunk * bits_per_pixel;
  chunk_ptrs[2] = chunk_ptrs[1] + chunk_stride_bytes;
  leds_remaining -= leds_in_chunk;

  // Chunk 4
  chunk_bits[3] = leds_remaining * bits_per_pixel;
  chunk_ptrs[3] = chunk_ptrs[2] + chunk_stride_bytes;

  unsigned long before = micros();
  ESP_ERROR_CHECK(parlio_tx_unit_wait_all_done(parlio_tx_unit, -1));
  unsigned long after = micros();

#ifdef WLEDMM_REMAP_AT_OUTPUT
  parallel_buffer_remapped = (parallel_buffer_remapped == parallel_buffer_remapped1) ? parallel_buffer_remapped2 : parallel_buffer_remapped1;
#endif
  parallel_buffer_repacked = (parallel_buffer_repacked == parallel_buffer_repacked1) ? parallel_buffer_repacked2 : parallel_buffer_repacked1;

  if (after - before < 50) delayMicroseconds(20);
  
  // portENTER_CRITICAL(&parlio_spinlock);
  for (int i = 0; i < num_chunks && i < 4; ++i) {
    ESP_ERROR_CHECK(parlio_tx_unit_transmit(parlio_tx_unit, chunk_ptrs[i], chunk_bits[i], &transmit_config));
  }
  // portEXIT_CRITICAL(&parlio_spinlock);

#ifdef PARLIO_TIMER
  if (micros() % 100 < 3) {
    USER_PRINTF("Parallel IO for %u pixels took %lu micros at %u FPS.\n", length, micros() - timer, strip.getFps());
  }
#endif

  return 0;
}
#endif