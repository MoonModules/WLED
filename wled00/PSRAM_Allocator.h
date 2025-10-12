#pragma once

#include "Arduino.h"
#include <cstdlib>
#include "esp_psram.h"

template <class T>
struct PSRAM_Allocator {
  typedef T value_type;
  PSRAM_Allocator() = default;
  template <class U> constexpr PSRAM_Allocator(const PSRAM_Allocator<U>&) noexcept { }
  [[nodiscard]] T* allocate(std::size_t n) {
    if (n > std::size_t(-1) / sizeof(T)) throw std::bad_alloc();
    // if (auto p = static_cast<T*>(ps_malloc(n * sizeof(T)))) return p;
    if (auto p = static_cast<T*>(heap_caps_malloc_prefer(n * sizeof(T), 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA | MALLOC_CAP_CACHE_ALIGNED, MALLOC_CAP_DMA | MALLOC_CAP_CACHE_ALIGNED))) return p;
    throw std::bad_alloc();
    }
  void deallocate(T* p, std::size_t) noexcept { heap_caps_free(p); }
  void deallocate(void* p) noexcept {
    heap_caps_free(p);
  }

  };
template <class T, class U>
bool operator==(const PSRAM_Allocator<T>&, const PSRAM_Allocator<U>&) { return true; }
template <class T, class U>
bool operator!=(const PSRAM_Allocator<T>&, const PSRAM_Allocator<U>&) { return false; }