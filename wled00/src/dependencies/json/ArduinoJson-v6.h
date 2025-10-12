// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2023, Benoit BLANCHON
// MIT License

#pragma once

#ifdef __cplusplus

#if __cplusplus < 201103L && (!defined(_MSC_VER) || _MSC_VER < 1910)
#  error ArduinoJson requires C++11 or newer. Configure your compiler for C++11 or downgrade ArduinoJson to 6.20.
#endif
#ifndef ARDUINOJSON_ENABLE_STD_STREAM
#  ifdef __has_include
#    if __has_include(<istream>) && \
    __has_include(<ostream>) && \
    !defined(min) && \
    !defined(max)
#      define ARDUINOJSON_ENABLE_STD_STREAM 1
#    else
#      define ARDUINOJSON_ENABLE_STD_STREAM 0
#    endif
#  else
#    ifdef ARDUINO
#      define ARDUINOJSON_ENABLE_STD_STREAM 0
#    else
#      define ARDUINOJSON_ENABLE_STD_STREAM 1
#    endif
#  endif
#endif
#ifndef ARDUINOJSON_ENABLE_STD_STRING
#  ifdef __has_include
#    if __has_include(<string>) && !defined(min) && !defined(max)
#      define ARDUINOJSON_ENABLE_STD_STRING 1
#    else
#      define ARDUINOJSON_ENABLE_STD_STRING 0
#    endif
#  else
#    ifdef ARDUINO
#      define ARDUINOJSON_ENABLE_STD_STRING 0
#    else
#      define ARDUINOJSON_ENABLE_STD_STRING 1
#    endif
#  endif
#endif
#ifndef ARDUINOJSON_ENABLE_STRING_VIEW
#  ifdef __has_include
#    if __has_include(<string_view>) && __cplusplus >= 201703L
#      define ARDUINOJSON_ENABLE_STRING_VIEW 1
#    else
#      define ARDUINOJSON_ENABLE_STRING_VIEW 0
#    endif
#  else
#    define ARDUINOJSON_ENABLE_STRING_VIEW 0
#  endif
#endif
#ifndef ARDUINOJSON_USE_DOUBLE
#  define ARDUINOJSON_USE_DOUBLE 1
#endif
#ifndef ARDUINOJSON_USE_LONG_LONG
#  if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ >= 4 || \
      defined(_MSC_VER)
#    define ARDUINOJSON_USE_LONG_LONG 1
#  endif
#endif
#ifndef ARDUINOJSON_USE_LONG_LONG
#  define ARDUINOJSON_USE_LONG_LONG 0
#endif
#ifndef ARDUINOJSON_DEFAULT_NESTING_LIMIT
#  define ARDUINOJSON_DEFAULT_NESTING_LIMIT 10
#endif
#ifndef ARDUINOJSON_SLOT_OFFSET_SIZE
#  if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ <= 2
#    define ARDUINOJSON_SLOT_OFFSET_SIZE 1
#  elif defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ >= 8 || \
      defined(_WIN64) && _WIN64
#    define ARDUINOJSON_SLOT_OFFSET_SIZE 4
#  else
#    define ARDUINOJSON_SLOT_OFFSET_SIZE 2
#  endif
#endif
#ifdef ARDUINO
#  ifndef ARDUINOJSON_ENABLE_ARDUINO_STRING
#    define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
#  endif
#  ifndef ARDUINOJSON_ENABLE_ARDUINO_STREAM
#    define ARDUINOJSON_ENABLE_ARDUINO_STREAM 1
#  endif
#  ifndef ARDUINOJSON_ENABLE_ARDUINO_PRINT
#    define ARDUINOJSON_ENABLE_ARDUINO_PRINT 1
#  endif
#  ifndef ARDUINOJSON_ENABLE_PROGMEM
#    define ARDUINOJSON_ENABLE_PROGMEM 1
#  endif
#else  // ARDUINO
#  ifndef ARDUINOJSON_ENABLE_ARDUINO_STRING
#    define ARDUINOJSON_ENABLE_ARDUINO_STRING 0
#  endif
#  ifndef ARDUINOJSON_ENABLE_ARDUINO_STREAM
#    define ARDUINOJSON_ENABLE_ARDUINO_STREAM 0
#  endif
#  ifndef ARDUINOJSON_ENABLE_ARDUINO_PRINT
#    define ARDUINOJSON_ENABLE_ARDUINO_PRINT 0
#  endif
#  ifndef ARDUINOJSON_ENABLE_PROGMEM
#    ifdef __AVR__
#      define ARDUINOJSON_ENABLE_PROGMEM 1
#    else
#      define ARDUINOJSON_ENABLE_PROGMEM 0
#    endif
#  endif
#endif  // ARDUINO
#ifndef ARDUINOJSON_DECODE_UNICODE
#  define ARDUINOJSON_DECODE_UNICODE 1
#endif
#ifndef ARDUINOJSON_ENABLE_COMMENTS
#  define ARDUINOJSON_ENABLE_COMMENTS 0
#endif
#ifndef ARDUINOJSON_ENABLE_NAN
#  define ARDUINOJSON_ENABLE_NAN 0
#endif
#ifndef ARDUINOJSON_ENABLE_INFINITY
#  define ARDUINOJSON_ENABLE_INFINITY 0
#endif
#ifndef ARDUINOJSON_POSITIVE_EXPONENTIATION_THRESHOLD
#  define ARDUINOJSON_POSITIVE_EXPONENTIATION_THRESHOLD 1e7
#endif
#ifndef ARDUINOJSON_NEGATIVE_EXPONENTIATION_THRESHOLD
#  define ARDUINOJSON_NEGATIVE_EXPONENTIATION_THRESHOLD 1e-5
#endif
#ifndef ARDUINOJSON_LITTLE_ENDIAN
#  if defined(_MSC_VER) ||                           \
      (defined(__BYTE_ORDER__) &&                    \
       __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) || \
      defined(__LITTLE_ENDIAN__) || defined(__i386) || defined(__x86_64)
#    define ARDUINOJSON_LITTLE_ENDIAN 1
#  else
#    define ARDUINOJSON_LITTLE_ENDIAN 0
#  endif
#endif
#ifndef ARDUINOJSON_ENABLE_ALIGNMENT
#  if defined(__AVR)
#    define ARDUINOJSON_ENABLE_ALIGNMENT 0
#  else
#    define ARDUINOJSON_ENABLE_ALIGNMENT 1
#  endif
#endif
#ifndef ARDUINOJSON_TAB
#  define ARDUINOJSON_TAB "  "
#endif
#ifndef ARDUINOJSON_ENABLE_STRING_DEDUPLICATION
#  define ARDUINOJSON_ENABLE_STRING_DEDUPLICATION 1
#endif
#ifndef ARDUINOJSON_STRING_BUFFER_SIZE
#  define ARDUINOJSON_STRING_BUFFER_SIZE 32
#endif
#ifndef ARDUINOJSON_DEBUG
#  ifdef __PLATFORMIO_BUILD_DEBUG__
#    define ARDUINOJSON_DEBUG 1
#  else
#    define ARDUINOJSON_DEBUG 0
#  endif
#endif
#if defined(nullptr)
#  error nullptr is defined as a macro. Remove the faulty #define or #undef nullptr
#endif
#if ARDUINOJSON_ENABLE_ARDUINO_STRING || ARDUINOJSON_ENABLE_ARDUINO_STREAM || \
    ARDUINOJSON_ENABLE_ARDUINO_PRINT ||                                       \
    (ARDUINOJSON_ENABLE_PROGMEM && defined(ARDUINO))
#include <Arduino.h>
#endif
#if !ARDUINOJSON_DEBUG
#  ifdef __clang__
#    pragma clang system_header
#  elif defined __GNUC__
#    pragma GCC system_header
#  endif
#endif
#define ARDUINOJSON_CONCAT_(A, B) A##B
#define ARDUINOJSON_CONCAT2(A, B) ARDUINOJSON_CONCAT_(A, B)
#define ARDUINOJSON_CONCAT4(A, B, C, D) \
  ARDUINOJSON_CONCAT2(ARDUINOJSON_CONCAT2(A, B), ARDUINOJSON_CONCAT2(C, D))
#define ARDUINOJSON_BIN2ALPHA_0000() A
#define ARDUINOJSON_BIN2ALPHA_0001() B
#define ARDUINOJSON_BIN2ALPHA_0010() C
#define ARDUINOJSON_BIN2ALPHA_0011() D
#define ARDUINOJSON_BIN2ALPHA_0100() E
#define ARDUINOJSON_BIN2ALPHA_0101() F
#define ARDUINOJSON_BIN2ALPHA_0110() G
#define ARDUINOJSON_BIN2ALPHA_0111() H
#define ARDUINOJSON_BIN2ALPHA_1000() I
#define ARDUINOJSON_BIN2ALPHA_1001() J
#define ARDUINOJSON_BIN2ALPHA_1010() K
#define ARDUINOJSON_BIN2ALPHA_1011() L
#define ARDUINOJSON_BIN2ALPHA_1100() M
#define ARDUINOJSON_BIN2ALPHA_1101() N
#define ARDUINOJSON_BIN2ALPHA_1110() O
#define ARDUINOJSON_BIN2ALPHA_1111() P
#define ARDUINOJSON_BIN2ALPHA_(A, B, C, D) ARDUINOJSON_BIN2ALPHA_##A##B##C##D()
#define ARDUINOJSON_BIN2ALPHA(A, B, C, D) ARDUINOJSON_BIN2ALPHA_(A, B, C, D)
#define ARDUINOJSON_VERSION "6.21.5"
#define ARDUINOJSON_VERSION_MAJOR 6
#define ARDUINOJSON_VERSION_MINOR 21
#define ARDUINOJSON_VERSION_REVISION 5
#define ARDUINOJSON_VERSION_MACRO V6215
#ifndef ARDUINOJSON_VERSION_NAMESPACE
#  define ARDUINOJSON_VERSION_NAMESPACE                                       \
    ARDUINOJSON_CONCAT4(                                                      \
        ARDUINOJSON_VERSION_MACRO,                                            \
        ARDUINOJSON_BIN2ALPHA(                                                \
            ARDUINOJSON_ENABLE_PROGMEM, ARDUINOJSON_USE_LONG_LONG,            \
            ARDUINOJSON_USE_DOUBLE, ARDUINOJSON_ENABLE_STRING_DEDUPLICATION), \
        ARDUINOJSON_BIN2ALPHA(                                                \
            ARDUINOJSON_ENABLE_NAN, ARDUINOJSON_ENABLE_INFINITY,              \
            ARDUINOJSON_ENABLE_COMMENTS, ARDUINOJSON_DECODE_UNICODE),         \
        ARDUINOJSON_SLOT_OFFSET_SIZE)
#endif
#define ARDUINOJSON_BEGIN_PUBLIC_NAMESPACE \
  namespace ArduinoJson {                  \
  inline namespace ARDUINOJSON_VERSION_NAMESPACE {
#define ARDUINOJSON_END_PUBLIC_NAMESPACE \
  }                                      \
  }
#define ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE        \
  namespace ArduinoJson {                          \
  inline namespace ARDUINOJSON_VERSION_NAMESPACE { \
  namespace detail {
#define ARDUINOJSON_END_PRIVATE_NAMESPACE \
  }                                       \
  }                                       \
  }
ARDUINOJSON_BEGIN_PUBLIC_NAMESPACE
template <typename T, typename Enable = void>
struct Converter;
ARDUINOJSON_END_PUBLIC_NAMESPACE
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <typename T1, typename T2>
class InvalidConversion;  // Error here? See https://arduinojson.org/v6/invalid-conversion/
template <typename T>
struct ConverterNeedsWriteableRef;
ARDUINOJSON_END_PRIVATE_NAMESPACE
#include <stddef.h>
#include <stdint.h>
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
#if ARDUINOJSON_ENABLE_ALIGNMENT
inline bool isAligned(size_t value) {
  const size_t mask = sizeof(void*) - 1;
  size_t addr = value;
  return (addr & mask) == 0;
}
inline size_t addPadding(size_t bytes) {
  const size_t mask = sizeof(void*) - 1;
  return (bytes + mask) & ~mask;
}
template <size_t bytes>
struct AddPadding {
  static const size_t mask = sizeof(void*) - 1;
  static const size_t value = (bytes + mask) & ~mask;
};
#else
inline bool isAligned(size_t) {
  return true;
}
inline size_t addPadding(size_t bytes) {
  return bytes;
}
template <size_t bytes>
struct AddPadding {
  static const size_t value = bytes;
};
#endif
template <typename T>
inline bool isAligned(T* ptr) {
  return isAligned(reinterpret_cast<size_t>(ptr));
}
template <typename T>
inline T* addPadding(T* p) {
  size_t address = addPadding(reinterpret_cast<size_t>(p));
  return reinterpret_cast<T*>(address);
}
ARDUINOJSON_END_PRIVATE_NAMESPACE
#if ARDUINOJSON_DEBUG
#include <assert.h>
#  define ARDUINOJSON_ASSERT(X) assert(X)
#else
#  define ARDUINOJSON_ASSERT(X) ((void)0)
#endif
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <size_t X, size_t Y, bool MaxIsX = (X > Y)>
struct Max {};
template <size_t X, size_t Y>
struct Max<X, Y, true> {
  static const size_t value = X;
};
template <size_t X, size_t Y>
struct Max<X, Y, false> {
  static const size_t value = Y;
};
template <bool Condition, class TrueType, class FalseType>
struct conditional {
  typedef TrueType type;
};
template <class TrueType, class FalseType>
struct conditional<false, TrueType, FalseType> {
  typedef FalseType type;
};
template <bool Condition, typename T = void>
struct enable_if {};
template <typename T>
struct enable_if<true, T> {
  typedef T type;
};
template <typename T, T v>
struct integral_constant {
  static const T value = v;
};
typedef integral_constant<bool, true> true_type;
typedef integral_constant<bool, false> false_type;
template <typename T>
struct is_array : false_type {};
template <typename T>
struct is_array<T[]> : true_type {};
template <typename T, size_t N>
struct is_array<T[N]> : true_type {};
template <typename T>
struct remove_reference {
  typedef T type;
};
template <typename T>
struct remove_reference<T&> {
  typedef T type;
};
template <typename TBase, typename TDerived>
class is_base_of {
 protected:  // <- to avoid GCC's "all member functions in class are private"
  static int probe(const TBase*);
  static char probe(...);
 public:
  static const bool value =
      sizeof(probe(reinterpret_cast<typename remove_reference<TDerived>::type*>(
          0))) == sizeof(int);
};
template <typename T>
T&& declval();
template <typename T>
struct is_class {
 protected:  // <- to avoid GCC's "all member functions in class are private"
  template <typename U>
  static int probe(void (U::*)(void));
  template <typename>
  static char probe(...);
 public:
  static const bool value = sizeof(probe<T>(0)) == sizeof(int);
};
template <typename T>
struct is_const : false_type {};
template <typename T>
struct is_const<const T> : true_type {};
ARDUINOJSON_END_PRIVATE_NAMESPACE
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4244)
#endif
#ifdef __ICCARM__
#pragma diag_suppress=Pa093
#endif
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <typename From, typename To>
struct is_convertible {
 protected:  // <- to avoid GCC's "all member functions in class are private"
  static int probe(To);
  static char probe(...);
  static From& from_;
 public:
  static const bool value = sizeof(probe(from_)) == sizeof(int);
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
#ifdef _MSC_VER
#  pragma warning(pop)
#endif
#ifdef __ICCARM__
#pragma diag_default=Pa093
#endif
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <typename T, typename U>
struct is_same : false_type {};
template <typename T>
struct is_same<T, T> : true_type {};
template <typename T>
struct remove_cv {
  typedef T type;
};
template <typename T>
struct remove_cv<const T> {
  typedef T type;
};
template <typename T>
struct remove_cv<volatile T> {
  typedef T type;
};
template <typename T>
struct remove_cv<const volatile T> {
  typedef T type;
};
template <class T>
struct is_floating_point
    : integral_constant<
          bool,  //
          is_same<float, typename remove_cv<T>::type>::value ||
              is_same<double, typename remove_cv<T>::type>::value> {};
template <typename T>
struct is_integral : integral_constant<bool,
    is_same<typename remove_cv<T>::type, signed char>::value ||
    is_same<typename remove_cv<T>::type, unsigned char>::value ||
    is_same<typename remove_cv<T>::type, signed short>::value ||
    is_same<typename remove_cv<T>::type, unsigned short>::value ||
    is_same<typename remove_cv<T>::type, signed int>::value ||
    is_same<typename remove_cv<T>::type, unsigned int>::value ||
    is_same<typename remove_cv<T>::type, signed long>::value ||
    is_same<typename remove_cv<T>::type, unsigned long>::value ||
    is_same<typename remove_cv<T>::type, signed long long>::value ||
    is_same<typename remove_cv<T>::type, unsigned long long>::value ||
    is_same<typename remove_cv<T>::type, char>::value ||
    is_same<typename remove_cv<T>::type, bool>::value> {};
template <typename T>
struct is_enum {
  static const bool value = is_convertible<T, int>::value &&
                            !is_class<T>::value && !is_integral<T>::value &&
                            !is_floating_point<T>::value;
};
template <typename T>
struct is_pointer : false_type {};
template <typename T>
struct is_pointer<T*> : true_type {};
template <typename T>
struct is_signed : integral_constant<bool,
    is_same<typename remove_cv<T>::type, char>::value ||
    is_same<typename remove_cv<T>::type, signed char>::value ||
    is_same<typename remove_cv<T>::type, signed short>::value ||
    is_same<typename remove_cv<T>::type, signed int>::value ||
    is_same<typename remove_cv<T>::type, signed long>::value ||
    is_same<typename remove_cv<T>::type, signed long long>::value ||
    is_same<typename remove_cv<T>::type, float>::value ||
    is_same<typename remove_cv<T>::type, double>::value> {};
template <typename T>
struct is_unsigned : integral_constant<bool,
    is_same<typename remove_cv<T>::type, unsigned char>::value ||
    is_same<typename remove_cv<T>::type, unsigned short>::value ||
    is_same<typename remove_cv<T>::type, unsigned int>::value ||
    is_same<typename remove_cv<T>::type, unsigned long>::value ||
    is_same<typename remove_cv<T>::type, unsigned long long>::value ||
    is_same<typename remove_cv<T>::type, bool>::value> {};
template <typename T>
struct type_identity {
  typedef T type;
};
template <typename T>
struct make_unsigned;
template <>
struct make_unsigned<char> : type_identity<unsigned char> {};
template <>
struct make_unsigned<signed char> : type_identity<unsigned char> {};
template <>
struct make_unsigned<unsigned char> : type_identity<unsigned char> {};
template <>
struct make_unsigned<signed short> : type_identity<unsigned short> {};
template <>
struct make_unsigned<unsigned short> : type_identity<unsigned short> {};
template <>
struct make_unsigned<signed int> : type_identity<unsigned int> {};
template <>
struct make_unsigned<unsigned int> : type_identity<unsigned int> {};
template <>
struct make_unsigned<signed long> : type_identity<unsigned long> {};
template <>
struct make_unsigned<unsigned long> : type_identity<unsigned long> {};
template <>
struct make_unsigned<signed long long> : type_identity<unsigned long long> {};
template <>
struct make_unsigned<unsigned long long> : type_identity<unsigned long long> {};
template <class = void>
struct make_void {
  typedef void type;
};
template <typename T>
struct remove_const {
  typedef T type;
};
template <typename T>
struct remove_const<const T> {
  typedef T type;
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
#include <string.h>
#ifdef _MSC_VER  // Visual Studio
#  define FORCE_INLINE  // __forceinline causes C4714 when returning std::string
#  define NO_INLINE __declspec(noinline)
#elif defined(__GNUC__)  // GCC or Clang
#  define FORCE_INLINE __attribute__((always_inline))
#  define NO_INLINE __attribute__((noinline))
#else  // Other compilers
#  define FORCE_INLINE
#  define NO_INLINE
#endif
#if defined(__has_attribute)
#  if __has_attribute(no_sanitize)
#    define ARDUINOJSON_NO_SANITIZE(check) __attribute__((no_sanitize(check)))
#  else
#    define ARDUINOJSON_NO_SANITIZE(check)
#  endif
#else
#  define ARDUINOJSON_NO_SANITIZE(check)
#endif
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
namespace StringStoragePolicy {
struct Link {};
struct Copy {};
struct LinkOrCopy {
  bool link;
};
}  // namespace StringStoragePolicy
template <typename TString, typename Enable = void>
struct StringAdapter;
template <typename TString, typename Enable = void>
struct SizedStringAdapter;
template <typename TString>
typename StringAdapter<TString>::AdaptedString adaptString(const TString& s) {
  return StringAdapter<TString>::adapt(s);
}
template <typename TChar>
typename StringAdapter<TChar*>::AdaptedString adaptString(TChar* p) {
  return StringAdapter<TChar*>::adapt(p);
}
template <typename TChar>
typename SizedStringAdapter<TChar*>::AdaptedString adaptString(TChar* p,
                                                               size_t n) {
  return SizedStringAdapter<TChar*>::adapt(p, n);
}
template <typename T>
struct IsChar
    : integral_constant<bool, is_integral<T>::value && sizeof(T) == 1> {};
class ZeroTerminatedRamString {
 public:
  static const size_t typeSortKey = 3;
  ZeroTerminatedRamString(const char* str) : str_(str) {}
  bool isNull() const {
    return !str_;
  }
  FORCE_INLINE size_t size() const {
    return str_ ? ::strlen(str_) : 0;
  }
  char operator[](size_t i) const {
    ARDUINOJSON_ASSERT(str_ != 0);
    ARDUINOJSON_ASSERT(i <= size());
    return str_[i];
  }
  const char* data() const {
    return str_;
  }
  friend int stringCompare(ZeroTerminatedRamString a,
                           ZeroTerminatedRamString b) {
    ARDUINOJSON_ASSERT(!a.isNull());
    ARDUINOJSON_ASSERT(!b.isNull());
    return ::strcmp(a.str_, b.str_);
  }
  friend bool stringEquals(ZeroTerminatedRamString a,
                           ZeroTerminatedRamString b) {
    return stringCompare(a, b) == 0;
  }
  StringStoragePolicy::Copy storagePolicy() const {
    return StringStoragePolicy::Copy();
  }
 protected:
  const char* str_;
};
template <typename TChar>
struct StringAdapter<TChar*, typename enable_if<IsChar<TChar>::value>::type> {
  typedef ZeroTerminatedRamString AdaptedString;
  static AdaptedString adapt(const TChar* p) {
    return AdaptedString(reinterpret_cast<const char*>(p));
  }
};
template <typename TChar, size_t N>
struct StringAdapter<TChar[N], typename enable_if<IsChar<TChar>::value>::type> {
  typedef ZeroTerminatedRamString AdaptedString;
  static AdaptedString adapt(const TChar* p) {
    return AdaptedString(reinterpret_cast<const char*>(p));
  }
};
class StaticStringAdapter : public ZeroTerminatedRamString {
 public:
  StaticStringAdapter(const char* str) : ZeroTerminatedRamString(str) {}
  StringStoragePolicy::Link storagePolicy() const {
    return StringStoragePolicy::Link();
  }
};
template <>
struct StringAdapter<const char*, void> {
  typedef StaticStringAdapter AdaptedString;
  static AdaptedString adapt(const char* p) {
    return AdaptedString(p);
  }
};
class SizedRamString {
 public:
  static const size_t typeSortKey = 2;
  SizedRamString(const char* str, size_t sz) : str_(str), size_(sz) {}
  bool isNull() const {
    return !str_;
  }
  size_t size() const {
    return size_;
  }
  char operator[](size_t i) const {
    ARDUINOJSON_ASSERT(str_ != 0);
    ARDUINOJSON_ASSERT(i <= size());
    return str_[i];
  }
  const char* data() const {
    return str_;
  }
  StringStoragePolicy::Copy storagePolicy() const {
    return StringStoragePolicy::Copy();
  }
 protected:
  const char* str_;
  size_t size_;
};
template <typename TChar>
struct SizedStringAdapter<TChar*,
                          typename enable_if<IsChar<TChar>::value>::type> {
  typedef SizedRamString AdaptedString;
  static AdaptedString adapt(const TChar* p, size_t n) {
    return AdaptedString(reinterpret_cast<const char*>(p), n);
  }
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
#if ARDUINOJSON_ENABLE_STD_STREAM
#include <ostream>
#endif
ARDUINOJSON_BEGIN_PUBLIC_NAMESPACE
class JsonString {
 public:
  enum Ownership { Copied, Linked };
  JsonString() : data_(0), size_(0), ownership_(Linked) {}
  JsonString(const char* data, Ownership ownership = Linked)
      : data_(data), size_(data ? ::strlen(data) : 0), ownership_(ownership) {}
  JsonString(const char* data, size_t sz, Ownership ownership = Linked)
      : data_(data), size_(sz), ownership_(ownership) {}
  const char* c_str() const {
    return data_;
  }
  bool isNull() const {
    return !data_;
  }
  bool isLinked() const {
    return ownership_ == Linked;
  }
  size_t size() const {
    return size_;
  }
  explicit operator bool() const {
    return data_ != 0;
  }
  friend bool operator==(JsonString lhs, JsonString rhs) {
    if (lhs.size_ != rhs.size_)
      return false;
    if (lhs.data_ == rhs.data_)
      return true;
    if (!lhs.data_)
      return false;
    if (!rhs.data_)
      return false;
    return memcmp(lhs.data_, rhs.data_, lhs.size_) == 0;
  }
  friend bool operator!=(JsonString lhs, JsonString rhs) {
    return !(lhs == rhs);
  }
#if ARDUINOJSON_ENABLE_STD_STREAM
  friend std::ostream& operator<<(std::ostream& lhs, const JsonString& rhs) {
    lhs.write(rhs.c_str(), static_cast<std::streamsize>(rhs.size()));
    return lhs;
  }
#endif
 private:
  const char* data_;
  size_t size_;
  Ownership ownership_;
};
ARDUINOJSON_END_PUBLIC_NAMESPACE
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
class JsonStringAdapter : public SizedRamString {
 public:
  JsonStringAdapter(const JsonString& s)
      : SizedRamString(s.c_str(), s.size()), linked_(s.isLinked()) {}
  StringStoragePolicy::LinkOrCopy storagePolicy() const {
    StringStoragePolicy::LinkOrCopy policy = {linked_};
    return policy;
  }
 private:
  bool linked_;
};
template <>
struct StringAdapter<JsonString> {
  typedef JsonStringAdapter AdaptedString;
  static AdaptedString adapt(const JsonString& s) {
    return AdaptedString(s);
  }
};
namespace string_traits_impl {
template <class T, class = void>
struct has_cstr : false_type {};
template <class T>
struct has_cstr<T,
                typename enable_if<is_same<decltype(declval<const T>().c_str()),
                                           const char*>::value>::type>
    : true_type {};
template <class T, class = void>
struct has_data : false_type {};
template <class T>
struct has_data<T,
                typename enable_if<is_same<decltype(declval<const T>().data()),
                                           const char*>::value>::type>
    : true_type {};
template <class T, class = void>
struct has_length : false_type {};
template <class T>
struct has_length<
    T, typename enable_if<
           is_same<decltype(declval<const T>().length()), size_t>::value>::type>
    : true_type {};
template <class T, class = void>
struct has_size : false_type {};
template <class T>
struct has_size<
    T, typename enable_if<
           is_same<decltype(declval<const T>().size()), size_t>::value>::type>
    : true_type {};
}  // namespace string_traits_impl
template <typename T>
struct string_traits {
  enum {
    has_cstr = string_traits_impl::has_cstr<T>::value,
    has_length = string_traits_impl::has_length<T>::value,
    has_data = string_traits_impl::has_data<T>::value,
    has_size = string_traits_impl::has_size<T>::value
  };
};
template <typename T>
struct StringAdapter<
    T,
    typename enable_if<
        (string_traits<T>::has_cstr || string_traits<T>::has_data) &&
        (string_traits<T>::has_length || string_traits<T>::has_size)>::type> {
  typedef SizedRamString AdaptedString;
  static AdaptedString adapt(const T& s) {
    return AdaptedString(get_data(s), get_size(s));
  }
 private:
  template <typename U>
  static typename enable_if<string_traits<U>::has_size, size_t>::type get_size(
      const U& s) {
    return s.size();
  }
  template <typename U>
  static typename enable_if<!string_traits<U>::has_size, size_t>::type get_size(
      const U& s) {
    return s.length();
  }
  template <typename U>
  static typename enable_if<string_traits<U>::has_data, const char*>::type
  get_data(const U& s) {
    return s.data();
  }
  template <typename U>
  static typename enable_if<!string_traits<U>::has_data, const char*>::type
  get_data(const U& s) {
    return s.c_str();
  }
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
#if ARDUINOJSON_ENABLE_PROGMEM
#ifdef ARDUINO
#else
class __FlashStringHelper;
#include <avr/pgmspace.h>
#endif
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
struct pgm_p {
  pgm_p(const void* p) : address(reinterpret_cast<const char*>(p)) {}
  const char* address;
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
#ifndef strlen_P
inline size_t strlen_P(ArduinoJson::detail::pgm_p s) {
  const char* p = s.address;
  ARDUINOJSON_ASSERT(p != NULL);
  while (pgm_read_byte(p))
    p++;
  return size_t(p - s.address);
}
#endif
#ifndef strncmp_P
inline int strncmp_P(const char* a, ArduinoJson::detail::pgm_p b, size_t n) {
  const char* s1 = a;
  const char* s2 = b.address;
  ARDUINOJSON_ASSERT(s1 != NULL);
  ARDUINOJSON_ASSERT(s2 != NULL);
  while (n-- > 0) {
    char c1 = *s1++;
    char c2 = static_cast<char>(pgm_read_byte(s2++));
    if (c1 < c2)
      return -1;
    if (c1 > c2)
      return 1;
    if (c1 == 0 /* and c2 as well */)
      return 0;
  }
  return 0;
}
#endif
#ifndef strcmp_P
inline int strcmp_P(const char* a, ArduinoJson::detail::pgm_p b) {
  const char* s1 = a;
  const char* s2 = b.address;
  ARDUINOJSON_ASSERT(s1 != NULL);
  ARDUINOJSON_ASSERT(s2 != NULL);
  for (;;) {
    char c1 = *s1++;
    char c2 = static_cast<char>(pgm_read_byte(s2++));
    if (c1 < c2)
      return -1;
    if (c1 > c2)
      return 1;
    if (c1 == 0 /* and c2 as well */)
      return 0;
  }
}
#endif
#ifndef memcmp_P
inline int memcmp_P(const void* a, ArduinoJson::detail::pgm_p b, size_t n) {
  const uint8_t* p1 = reinterpret_cast<const uint8_t*>(a);
  const char* p2 = b.address;
  ARDUINOJSON_ASSERT(p1 != NULL);
  ARDUINOJSON_ASSERT(p2 != NULL);
  while (n-- > 0) {
    uint8_t v1 = *p1++;
    uint8_t v2 = pgm_read_byte(p2++);
    if (v1 != v2)
      return v1 - v2;
  }
  return 0;
}
#endif
#ifndef memcpy_P
inline void* memcpy_P(void* dst, ArduinoJson::detail::pgm_p src, size_t n) {
  uint8_t* d = reinterpret_cast<uint8_t*>(dst);
  const char* s = src.address;
  ARDUINOJSON_ASSERT(d != NULL);
  ARDUINOJSON_ASSERT(s != NULL);
  while (n-- > 0) {
    *d++ = pgm_read_byte(s++);
  }
  return dst;
}
#endif
#ifndef pgm_read_dword
inline uint32_t pgm_read_dword(ArduinoJson::detail::pgm_p p) {
  uint32_t result;
  memcpy_P(&result, p.address, 4);
  return result;
}
#endif
#ifndef pgm_read_float
inline float pgm_read_float(ArduinoJson::detail::pgm_p p) {
  float result;
  memcpy_P(&result, p.address, sizeof(float));
  return result;
}
#endif
#ifndef pgm_read_double
#  if defined(__SIZEOF_DOUBLE__) && defined(__SIZEOF_FLOAT__) && \
      __SIZEOF_DOUBLE__ == __SIZEOF_FLOAT__
inline double pgm_read_double(ArduinoJson::detail::pgm_p p) {
  return pgm_read_float(p.address);
}
#  else
inline double pgm_read_double(ArduinoJson::detail::pgm_p p) {
  double result;
  memcpy_P(&result, p.address, sizeof(double));
  return result;
}
#  endif
#endif
#ifndef pgm_read_ptr
inline void* pgm_read_ptr(ArduinoJson::detail::pgm_p p) {
  void* result;
  memcpy_P(&result, p.address, sizeof(result));
  return result;
}
#endif
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
class FlashString {
 public:
  static const size_t typeSortKey = 1;
  FlashString(const __FlashStringHelper* str, size_t sz)
      : str_(reinterpret_cast<const char*>(str)), size_(sz) {}
  bool isNull() const {
    return !str_;
  }
  char operator[](size_t i) const {
    ARDUINOJSON_ASSERT(str_ != 0);
    ARDUINOJSON_ASSERT(i <= size_);
    return static_cast<char>(pgm_read_byte(str_ + i));
  }
  size_t size() const {
    return size_;
  }
  friend bool stringEquals(FlashString a, SizedRamString b) {
    ARDUINOJSON_ASSERT(a.typeSortKey < b.typeSortKey);
    ARDUINOJSON_ASSERT(!a.isNull());
    ARDUINOJSON_ASSERT(!b.isNull());
    if (a.size() != b.size())
      return false;
    return ::memcmp_P(b.data(), a.str_, a.size_) == 0;
  }
  friend int stringCompare(FlashString a, SizedRamString b) {
    ARDUINOJSON_ASSERT(a.typeSortKey < b.typeSortKey);
    ARDUINOJSON_ASSERT(!a.isNull());
    ARDUINOJSON_ASSERT(!b.isNull());
    size_t minsize = a.size() < b.size() ? a.size() : b.size();
    int res = ::memcmp_P(b.data(), a.str_, minsize);
    if (res)
      return -res;
    if (a.size() < b.size())
      return -1;
    if (a.size() > b.size())
      return 1;
    return 0;
  }
  friend void stringGetChars(FlashString s, char* p, size_t n) {
    ARDUINOJSON_ASSERT(s.size() <= n);
    ::memcpy_P(p, s.str_, n);
  }
  StringStoragePolicy::Copy storagePolicy() const {
    return StringStoragePolicy::Copy();
  }
 private:
  const char* str_;
  size_t size_;
};
template <>
struct StringAdapter<const __FlashStringHelper*, void> {
  typedef FlashString AdaptedString;
  static AdaptedString adapt(const __FlashStringHelper* s) {
    return AdaptedString(s, s ? strlen_P(reinterpret_cast<const char*>(s)) : 0);
  }
};
template <>
struct SizedStringAdapter<const __FlashStringHelper*, void> {
  typedef FlashString AdaptedString;
  static AdaptedString adapt(const __FlashStringHelper* s, size_t n) {
    return AdaptedString(s, n);
  }
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
#endif
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <typename TAdaptedString1, typename TAdaptedString2>
typename enable_if<TAdaptedString1::typeSortKey <= TAdaptedString2::typeSortKey,
                   int>::type
stringCompare(TAdaptedString1 s1, TAdaptedString2 s2) {
  ARDUINOJSON_ASSERT(!s1.isNull());
  ARDUINOJSON_ASSERT(!s2.isNull());
  size_t size1 = s1.size();
  size_t size2 = s2.size();
  size_t n = size1 < size2 ? size1 : size2;
  for (size_t i = 0; i < n; i++) {
    if (s1[i] != s2[i])
      return s1[i] - s2[i];
  }
  if (size1 < size2)
    return -1;
  if (size1 > size2)
    return 1;
  return 0;
}
template <typename TAdaptedString1, typename TAdaptedString2>
typename enable_if<
    (TAdaptedString1::typeSortKey > TAdaptedString2::typeSortKey), int>::type
stringCompare(TAdaptedString1 s1, TAdaptedString2 s2) {
  return -stringCompare(s2, s1);
}
template <typename TAdaptedString1, typename TAdaptedString2>
typename enable_if<TAdaptedString1::typeSortKey <= TAdaptedString2::typeSortKey,
                   bool>::type
stringEquals(TAdaptedString1 s1, TAdaptedString2 s2) {
  ARDUINOJSON_ASSERT(!s1.isNull());
  ARDUINOJSON_ASSERT(!s2.isNull());
  size_t size1 = s1.size();
  size_t size2 = s2.size();
  if (size1 != size2)
    return false;
  for (size_t i = 0; i < size1; i++) {
    if (s1[i] != s2[i])
      return false;
  }
  return true;
}
template <typename TAdaptedString1, typename TAdaptedString2>
typename enable_if<
    (TAdaptedString1::typeSortKey > TAdaptedString2::typeSortKey), bool>::type
stringEquals(TAdaptedString1 s1, TAdaptedString2 s2) {
  return stringEquals(s2, s1);
}
template <typename TAdaptedString>
static void stringGetChars(TAdaptedString s, char* p, size_t n) {
  ARDUINOJSON_ASSERT(s.size() <= n);
  for (size_t i = 0; i < n; i++) {
    p[i] = s[i];
  }
}
template <int Bits>
struct int_t;
template <>
struct int_t<8> {
  typedef int8_t type;
};
template <>
struct int_t<16> {
  typedef int16_t type;
};
template <>
struct int_t<32> {
  typedef int32_t type;
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4310)
#endif
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <typename T, typename Enable = void>
struct numeric_limits;
template <typename T>
struct numeric_limits<T, typename enable_if<is_unsigned<T>::value>::type> {
  static T lowest() {
    return 0;
  }
  static T highest() {
    return T(-1);
  }
};
template <typename T>
struct numeric_limits<
    T, typename enable_if<is_integral<T>::value && is_signed<T>::value>::type> {
  static T lowest() {
    return T(T(1) << (sizeof(T) * 8 - 1));
  }
  static T highest() {
    return T(~lowest());
  }
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
#ifdef _MSC_VER
#  pragma warning(pop)
#endif
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
class MemoryPool;
class VariantData;
class VariantSlot;
class CollectionData {
  VariantSlot* head_;
  VariantSlot* tail_;
 public:
  VariantData* addElement(MemoryPool* pool);
  VariantData* getElement(size_t index) const;
  VariantData* getOrAddElement(size_t index, MemoryPool* pool);
  void removeElement(size_t index);
  template <typename TAdaptedString>
  VariantData* addMember(TAdaptedString key, MemoryPool* pool);
  template <typename TAdaptedString>
  VariantData* getMember(TAdaptedString key) const;
  template <typename TAdaptedString>
  VariantData* getOrAddMember(TAdaptedString key, MemoryPool* pool);
  template <typename TAdaptedString>
  void removeMember(TAdaptedString key) {
    removeSlot(getSlot(key));
  }
  template <typename TAdaptedString>
  bool containsKey(const TAdaptedString& key) const;
  void clear();
  size_t memoryUsage() const;
  size_t size() const;
  VariantSlot* addSlot(MemoryPool*);
  void removeSlot(VariantSlot* slot);
  bool copyFrom(const CollectionData& src, MemoryPool* pool);
  VariantSlot* head() const {
    return head_;
  }
  void movePointers(ptrdiff_t stringDistance, ptrdiff_t variantDistance);
 private:
  VariantSlot* getSlot(size_t index) const;
  template <typename TAdaptedString>
  VariantSlot* getSlot(TAdaptedString key) const;
  VariantSlot* getPreviousSlot(VariantSlot*) const;
};
inline const VariantData* collectionToVariant(
    const CollectionData* collection) {
  const void* data = collection;  // prevent warning cast-align
  return reinterpret_cast<const VariantData*>(data);
}
inline VariantData* collectionToVariant(CollectionData* collection) {
  void* data = collection;  // prevent warning cast-align
  return reinterpret_cast<VariantData*>(data);
}
ARDUINOJSON_END_PRIVATE_NAMESPACE
ARDUINOJSON_BEGIN_PUBLIC_NAMESPACE
#if ARDUINOJSON_USE_DOUBLE
typedef double JsonFloat;
#else
typedef float JsonFloat;
#endif
#if ARDUINOJSON_USE_LONG_LONG
typedef int64_t JsonInteger;
typedef uint64_t JsonUInt;
#else
typedef long JsonInteger;
typedef unsigned long JsonUInt;
#endif
ARDUINOJSON_END_PUBLIC_NAMESPACE
#define ARDUINOJSON_ASSERT_INTEGER_TYPE_IS_SUPPORTED(T)                  \
  static_assert(sizeof(T) <= sizeof(ArduinoJson::JsonInteger),           \
                "To use 64-bit integers with ArduinoJson, you must set " \
                "ARDUINOJSON_USE_LONG_LONG to 1. See "                   \
                "https://arduinojson.org/v6/api/config/use_long_long/");
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
enum {
  VALUE_MASK = 0x7F,
  OWNED_VALUE_BIT = 0x01,
  VALUE_IS_NULL = 0,
  VALUE_IS_LINKED_RAW = 0x02,
  VALUE_IS_OWNED_RAW = 0x03,
  VALUE_IS_LINKED_STRING = 0x04,
  VALUE_IS_OWNED_STRING = 0x05,
  VALUE_IS_BOOLEAN = 0x06,
  NUMBER_BIT = 0x08,
  VALUE_IS_UNSIGNED_INTEGER = 0x08,
  VALUE_IS_SIGNED_INTEGER = 0x0A,
  VALUE_IS_FLOAT = 0x0C,
  COLLECTION_MASK = 0x60,
  VALUE_IS_OBJECT = 0x20,
  VALUE_IS_ARRAY = 0x40,
  OWNED_KEY_BIT = 0x80
};
struct RawData {
  const char* data;
  size_t size;
};
union VariantContent {
  JsonFloat asFloat;
  bool asBoolean;
  JsonUInt asUnsignedInteger;
  JsonInteger asSignedInteger;
  CollectionData asCollection;
  struct {
    const char* data;
    size_t size;
  } asString;
};
typedef int_t<ARDUINOJSON_SLOT_OFFSET_SIZE * 8>::type VariantSlotDiff;
class VariantSlot {
  VariantContent content_;
  uint8_t flags_;
  VariantSlotDiff next_;
  const char* key_;
 public:
  VariantData* data() {
    return reinterpret_cast<VariantData*>(&content_);
  }
  const VariantData* data() const {
    return reinterpret_cast<const VariantData*>(&content_);
  }
  VariantSlot* next() {
    return next_ ? this + next_ : 0;
  }
  const VariantSlot* next() const {
    return const_cast<VariantSlot*>(this)->next();
  }
  VariantSlot* next(size_t distance) {
    VariantSlot* slot = this;
    while (distance--) {
      if (!slot->next_)
        return 0;
      slot += slot->next_;
    }
    return slot;
  }
  const VariantSlot* next(size_t distance) const {
    return const_cast<VariantSlot*>(this)->next(distance);
  }
  void setNext(VariantSlot* slot) {
    ARDUINOJSON_ASSERT(!slot || slot - this >=
                                    numeric_limits<VariantSlotDiff>::lowest());
    ARDUINOJSON_ASSERT(!slot || slot - this <=
                                    numeric_limits<VariantSlotDiff>::highest());
    next_ = VariantSlotDiff(slot ? slot - this : 0);
  }
  void setNextNotNull(VariantSlot* slot) {
    ARDUINOJSON_ASSERT(slot != 0);
    ARDUINOJSON_ASSERT(slot - this >=
                       numeric_limits<VariantSlotDiff>::lowest());
    ARDUINOJSON_ASSERT(slot - this <=
                       numeric_limits<VariantSlotDiff>::highest());
    next_ = VariantSlotDiff(slot - this);
  }
  void setKey(JsonString k) {
    ARDUINOJSON_ASSERT(k);
    if (k.isLinked())
      flags_ &= VALUE_MASK;
    else
      flags_ |= OWNED_KEY_BIT;
    key_ = k.c_str();
  }
  const char* key() const {
    return key_;
  }
  bool ownsKey() const {
    return (flags_ & OWNED_KEY_BIT) != 0;
  }
  void clear() {
    next_ = 0;
    flags_ = 0;
    key_ = 0;
  }
  void movePointers(ptrdiff_t stringDistance, ptrdiff_t variantDistance) {
    if (flags_ & OWNED_KEY_BIT)
      key_ += stringDistance;
    if (flags_ & OWNED_VALUE_BIT)
      content_.asString.data += stringDistance;
    if (flags_ & COLLECTION_MASK)
      content_.asCollection.movePointers(stringDistance, variantDistance);
  }
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
#define JSON_STRING_SIZE(SIZE) (SIZE + 1)
#define JSON_ARRAY_SIZE(NUMBER_OF_ELEMENTS) \
  ((NUMBER_OF_ELEMENTS) * sizeof(ArduinoJson::detail::VariantSlot))
#define JSON_OBJECT_SIZE(NUMBER_OF_ELEMENTS) \
  ((NUMBER_OF_ELEMENTS) * sizeof(ArduinoJson::detail::VariantSlot))
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
class MemoryPool {
 public:
  MemoryPool(char* buf, size_t capa)
      : begin_(buf),
        left_(buf),
        right_(buf ? buf + capa : 0),
        end_(buf ? buf + capa : 0),
        overflowed_(false) {
    ARDUINOJSON_ASSERT(isAligned(begin_));
    ARDUINOJSON_ASSERT(isAligned(right_));
    ARDUINOJSON_ASSERT(isAligned(end_));
  }
  void* buffer() {
    return begin_;  // NOLINT(clang-analyzer-unix.Malloc)
  }
  size_t capacity() const {
    return size_t(end_ - begin_);
  }
  size_t size() const {
    return size_t(left_ - begin_ + end_ - right_);
  }
  bool overflowed() const {
    return overflowed_;
  }
  VariantSlot* allocVariant() {
    return allocRight<VariantSlot>();
  }
  template <typename TAdaptedString>
  const char* saveString(TAdaptedString str) {
    if (str.isNull())
      return 0;
#if ARDUINOJSON_ENABLE_STRING_DEDUPLICATION
    const char* existingCopy = findString(str);
    if (existingCopy)
      return existingCopy;
#endif
    size_t n = str.size();
    char* newCopy = allocString(n + 1);
    if (newCopy) {
      stringGetChars(str, newCopy, n);
      newCopy[n] = 0;  // force null-terminator
    }
    return newCopy;
  }
  void getFreeZone(char** zoneStart, size_t* zoneSize) const {
    *zoneStart = left_;
    *zoneSize = size_t(right_ - left_);
  }
  const char* saveStringFromFreeZone(size_t len) {
#if ARDUINOJSON_ENABLE_STRING_DEDUPLICATION
    const char* dup = findString(adaptString(left_, len));
    if (dup)
      return dup;
#endif
    const char* str = left_;
    left_ += len;
    *left_++ = 0;
    checkInvariants();
    return str;
  }
  void markAsOverflowed() {
    overflowed_ = true;
  }
  void clear() {
    left_ = begin_;
    right_ = end_;
    overflowed_ = false;
  }
  bool canAlloc(size_t bytes) const {
    return left_ + bytes <= right_;
  }
  bool owns(void* p) const {
    return begin_ <= p && p < end_;
  }
  void* operator new(size_t, void* p) {
    return p;
  }
  ptrdiff_t squash() {
    char* new_right = addPadding(left_);
    if (new_right >= right_)
      return 0;
    size_t right_size = static_cast<size_t>(end_ - right_);
    memmove(new_right, right_, right_size);
    ptrdiff_t bytes_reclaimed = right_ - new_right;
    right_ = new_right;
    end_ = new_right + right_size;
    return bytes_reclaimed;
  }
  void movePointers(ptrdiff_t offset) {
    begin_ += offset;
    left_ += offset;
    right_ += offset;
    end_ += offset;
  }
 private:
  void checkInvariants() {
    ARDUINOJSON_ASSERT(begin_ <= left_);
    ARDUINOJSON_ASSERT(left_ <= right_);
    ARDUINOJSON_ASSERT(right_ <= end_);
    ARDUINOJSON_ASSERT(isAligned(right_));
  }
#if ARDUINOJSON_ENABLE_STRING_DEDUPLICATION
  template <typename TAdaptedString>
  const char* findString(const TAdaptedString& str) const {
    size_t n = str.size();
    for (char* next = begin_; next + n < left_; ++next) {
      if (next[n] == '\0' && stringEquals(str, adaptString(next, n)))
        return next;
      while (*next)
        ++next;
    }
    return 0;
  }
#endif
  char* allocString(size_t n) {
    if (!canAlloc(n)) {
      overflowed_ = true;
      return 0;
    }
    char* s = left_;
    left_ += n;
    checkInvariants();
    return s;
  }
  template <typename T>
  T* allocRight() {
    return reinterpret_cast<T*>(allocRight(sizeof(T)));
  }
  void* allocRight(size_t bytes) {
    if (!canAlloc(bytes)) {
      overflowed_ = true;
      return 0;
    }
    right_ -= bytes;
    return right_;
  }
  char *begin_, *left_, *right_, *end_;
  bool overflowed_;
};
template <typename TAdaptedString, typename TCallback>
bool storeString(MemoryPool* pool, TAdaptedString str,
                 StringStoragePolicy::Copy, TCallback callback) {
  const char* copy = pool->saveString(str);
  JsonString storedString(copy, str.size(), JsonString::Copied);
  callback(storedString);
  return copy != 0;
}
template <typename TAdaptedString, typename TCallback>
bool storeString(MemoryPool*, TAdaptedString str, StringStoragePolicy::Link,
                 TCallback callback) {
  JsonString storedString(str.data(), str.size(), JsonString::Linked);
  callback(storedString);
  return !str.isNull();
}
template <typename TAdaptedString, typename TCallback>
bool storeString(MemoryPool* pool, TAdaptedString str,
                 StringStoragePolicy::LinkOrCopy policy, TCallback callback) {
  if (policy.link)
    return storeString(pool, str, StringStoragePolicy::Link(), callback);
  else
    return storeString(pool, str, StringStoragePolicy::Copy(), callback);
}
template <typename TAdaptedString, typename TCallback>
bool storeString(MemoryPool* pool, TAdaptedString str, TCallback callback) {
  return storeString(pool, str, str.storagePolicy(), callback);
}
template <typename T, typename Enable = void>
struct IsString : false_type {};
template <typename T>
struct IsString<
    T, typename make_void<typename StringAdapter<T>::AdaptedString>::type>
    : true_type {};
ARDUINOJSON_END_PRIVATE_NAMESPACE
ARDUINOJSON_BEGIN_PUBLIC_NAMESPACE
class JsonArray;
class JsonObject;
class JsonVariant;
ARDUINOJSON_END_PUBLIC_NAMESPACE
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <typename T>
struct VariantTo {};
template <>
struct VariantTo<JsonArray> {
  typedef JsonArray type;
};
template <>
struct VariantTo<JsonObject> {
  typedef JsonObject type;
};
template <>
struct VariantTo<JsonVariant> {
  typedef JsonVariant type;
};
class VariantAttorney {
  template <typename TClient>
  struct ResultOfGetData {
   protected:  // <- to avoid GCC's "all member functions in class are private"
    static int probe(const VariantData*);
    static char probe(VariantData*);
    static TClient& client;
   public:
    typedef typename conditional<sizeof(probe(client.getData())) == sizeof(int),
                                 const VariantData*, VariantData*>::type type;
  };
 public:
  template <typename TClient>
  FORCE_INLINE static MemoryPool* getPool(TClient& client) {
    return client.getPool();
  }
  template <typename TClient>
  FORCE_INLINE static typename ResultOfGetData<TClient>::type getData(
      TClient& client) {
    return client.getData();
  }
  template <typename TClient>
  FORCE_INLINE static VariantData* getOrCreateData(TClient& client) {
    return client.getOrCreateData();
  }
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
ARDUINOJSON_BEGIN_PUBLIC_NAMESPACE
template <typename T>
class SerializedValue {
 public:
  explicit SerializedValue(T str) : str_(str) {}
  operator T() const {
    return str_;
  }
  const char* data() const {
    return str_.c_str();
  }
  size_t size() const {
    return str_.length();
  }
 private:
  T str_;
};
template <typename TChar>
class SerializedValue<TChar*> {
 public:
  explicit SerializedValue(TChar* p, size_t n) : data_(p), size_(n) {}
  operator TChar*() const {
    return data_;
  }
  TChar* data() const {
    return data_;
  }
  size_t size() const {
    return size_;
  }
 private:
  TChar* data_;
  size_t size_;
};
template <typename T>
inline SerializedValue<T> serialized(T str) {
  return SerializedValue<T>(str);
}
template <typename TChar>
inline SerializedValue<TChar*> serialized(TChar* p) {
  return SerializedValue<TChar*>(p, detail::adaptString(p).size());
}
template <typename TChar>
inline SerializedValue<TChar*> serialized(TChar* p, size_t n) {
  return SerializedValue<TChar*>(p, n);
}
ARDUINOJSON_END_PUBLIC_NAMESPACE
#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wconversion"
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <stdlib.h>
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
#ifndef isnan
template <typename T>
bool isnan(T x) {
  return x != x;
}
#endif
#ifndef isinf
template <typename T>
bool isinf(T x) {
  return x != 0.0 && x * 2 == x;
}
#endif
template <typename T, typename F>
struct alias_cast_t {
  union {
    F raw;
    T data;
  };
};
template <typename T, typename F>
T alias_cast(F raw_data) {
  alias_cast_t<T, F> ac;
  ac.raw = raw_data;
  return ac.data;
}
ARDUINOJSON_END_PRIVATE_NAMESPACE
#if ARDUINOJSON_ENABLE_PROGMEM
#endif
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
#if ARDUINOJSON_ENABLE_PROGMEM
#  ifndef ARDUINOJSON_DEFINE_PROGMEM_ARRAY
#    define ARDUINOJSON_DEFINE_PROGMEM_ARRAY(type, name, ...) \
      static type const name[] PROGMEM = __VA_ARGS__;
#  endif
template <typename T>
inline const T* pgm_read(const T* const* p) {
  return reinterpret_cast<const T*>(pgm_read_ptr(p));
}
inline uint32_t pgm_read(const uint32_t* p) {
  return pgm_read_dword(p);
}
inline double pgm_read(const double* p) {
  return pgm_read_double(p);
}
inline float pgm_read(const float* p) {
  return pgm_read_float(p);
}
#else
#  ifndef ARDUINOJSON_DEFINE_PROGMEM_ARRAY
#    define ARDUINOJSON_DEFINE_PROGMEM_ARRAY(type, name, ...) \
      static type const name[] = __VA_ARGS__;
#  endif
template <typename T>
inline T pgm_read(const T* p) {
  return *p;
}
#endif
template <typename T>
class pgm_ptr {
 public:
  explicit pgm_ptr(const T* ptr) : ptr_(ptr) {}
  T operator[](intptr_t index) const {
    return pgm_read(ptr_ + index);
  }
 private:
  const T* ptr_;
};
template <typename T, size_t = sizeof(T)>
struct FloatTraits {};
template <typename T>
struct FloatTraits<T, 8 /*64bits*/> {
  typedef uint64_t mantissa_type;
  static const short mantissa_bits = 52;
  static const mantissa_type mantissa_max =
      (mantissa_type(1) << mantissa_bits) - 1;
  typedef int16_t exponent_type;
  static const exponent_type exponent_max = 308;
  static pgm_ptr<T> positiveBinaryPowersOfTen() {
    ARDUINOJSON_DEFINE_PROGMEM_ARRAY(  //
        uint64_t, factors,
        {
            0x4024000000000000,  // 1e1
            0x4059000000000000,  // 1e2
            0x40C3880000000000,  // 1e4
            0x4197D78400000000,  // 1e8
            0x4341C37937E08000,  // 1e16
            0x4693B8B5B5056E17,  // 1e32
            0x4D384F03E93FF9F5,  // 1e64
            0x5A827748F9301D32,  // 1e128
            0x75154FDD7F73BF3C,  // 1e256
        });
    return pgm_ptr<T>(reinterpret_cast<const T*>(factors));
  }
  static pgm_ptr<T> negativeBinaryPowersOfTen() {
    ARDUINOJSON_DEFINE_PROGMEM_ARRAY(  //
        uint64_t, factors,
        {
            0x3FB999999999999A,  // 1e-1
            0x3F847AE147AE147B,  // 1e-2
            0x3F1A36E2EB1C432D,  // 1e-4
            0x3E45798EE2308C3A,  // 1e-8
            0x3C9CD2B297D889BC,  // 1e-16
            0x3949F623D5A8A733,  // 1e-32
            0x32A50FFD44F4A73D,  // 1e-64
            0x255BBA08CF8C979D,  // 1e-128
            0x0AC8062864AC6F43   // 1e-256
        });
    return pgm_ptr<T>(reinterpret_cast<const T*>(factors));
  }
  static T nan() {
    return forge(0x7ff8000000000000);
  }
  static T inf() {
    return forge(0x7ff0000000000000);
  }
  static T highest() {
    return forge(0x7FEFFFFFFFFFFFFF);
  }
  template <typename TOut>  // int64_t
  static T highest_for(
      typename enable_if<is_integral<TOut>::value && is_signed<TOut>::value &&
                             sizeof(TOut) == 8,
                         signed>::type* = 0) {
    return forge(0x43DFFFFFFFFFFFFF);  //  9.2233720368547748e+18
  }
  template <typename TOut>  // uint64_t
  static T highest_for(
      typename enable_if<is_integral<TOut>::value && is_unsigned<TOut>::value &&
                             sizeof(TOut) == 8,
                         unsigned>::type* = 0) {
    return forge(0x43EFFFFFFFFFFFFF);  //  1.8446744073709549568e+19
  }
  static T lowest() {
    return forge(0xFFEFFFFFFFFFFFFF);
  }
  static T forge(uint64_t bits) {
    return alias_cast<T>(bits);
  }
};
template <typename T>
struct FloatTraits<T, 4 /*32bits*/> {
  typedef uint32_t mantissa_type;
  static const short mantissa_bits = 23;
  static const mantissa_type mantissa_max =
      (mantissa_type(1) << mantissa_bits) - 1;
  typedef int8_t exponent_type;
  static const exponent_type exponent_max = 38;
  static pgm_ptr<T> positiveBinaryPowersOfTen() {
    ARDUINOJSON_DEFINE_PROGMEM_ARRAY(uint32_t, factors,
                                     {
                                         0x41200000,  // 1e1f
                                         0x42c80000,  // 1e2f
                                         0x461c4000,  // 1e4f
                                         0x4cbebc20,  // 1e8f
                                         0x5a0e1bca,  // 1e16f
                                         0x749dc5ae   // 1e32f
                                     });
    return pgm_ptr<T>(reinterpret_cast<const T*>(factors));
  }
  static pgm_ptr<T> negativeBinaryPowersOfTen() {
    ARDUINOJSON_DEFINE_PROGMEM_ARRAY(uint32_t, factors,
                                     {
                                         0x3dcccccd,  // 1e-1f
                                         0x3c23d70a,  // 1e-2f
                                         0x38d1b717,  // 1e-4f
                                         0x322bcc77,  // 1e-8f
                                         0x24e69595,  // 1e-16f
                                         0x0a4fb11f   // 1e-32f
                                     });
    return pgm_ptr<T>(reinterpret_cast<const T*>(factors));
  }
  static T forge(uint32_t bits) {
    return alias_cast<T>(bits);
  }
  static T nan() {
    return forge(0x7fc00000);
  }
  static T inf() {
    return forge(0x7f800000);
  }
  static T highest() {
    return forge(0x7f7fffff);
  }
  template <typename TOut>  // int32_t
  static T highest_for(
      typename enable_if<is_integral<TOut>::value && is_signed<TOut>::value &&
                             sizeof(TOut) == 4,
                         signed>::type* = 0) {
    return forge(0x4EFFFFFF);  // 2.14748352E9
  }
  template <typename TOut>  // uint32_t
  static T highest_for(
      typename enable_if<is_integral<TOut>::value && is_unsigned<TOut>::value &&
                             sizeof(TOut) == 4,
                         unsigned>::type* = 0) {
    return forge(0x4F7FFFFF);  // 4.29496704E9
  }
  template <typename TOut>  // int64_t
  static T highest_for(
      typename enable_if<is_integral<TOut>::value && is_signed<TOut>::value &&
                             sizeof(TOut) == 8,
                         signed>::type* = 0) {
    return forge(0x5EFFFFFF);  // 9.22337148709896192E18
  }
  template <typename TOut>  // uint64_t
  static T highest_for(
      typename enable_if<is_integral<TOut>::value && is_unsigned<TOut>::value &&
                             sizeof(TOut) == 8,
                         unsigned>::type* = 0) {
    return forge(0x5F7FFFFF);  // 1.844674297419792384E19
  }
  static T lowest() {
    return forge(0xFf7fffff);
  }
};
template <typename TFloat, typename TExponent>
inline TFloat make_float(TFloat m, TExponent e) {
  using traits = FloatTraits<TFloat>;
  auto powersOfTen = e > 0 ? traits::positiveBinaryPowersOfTen()
                           : traits::negativeBinaryPowersOfTen();
  if (e <= 0)
    e = TExponent(-e);
  for (uint8_t index = 0; e != 0; index++) {
    if (e & 1)
      m *= powersOfTen[index];
    e >>= 1;
  }
  return m;
}
template <typename TOut, typename TIn>
typename enable_if<is_integral<TIn>::value && is_unsigned<TIn>::value &&
                       is_integral<TOut>::value && sizeof(TOut) <= sizeof(TIn),
                   bool>::type
canConvertNumber(TIn value) {
  return value <= TIn(numeric_limits<TOut>::highest());
}
template <typename TOut, typename TIn>
typename enable_if<is_integral<TIn>::value && is_unsigned<TIn>::value &&
                       is_integral<TOut>::value && sizeof(TIn) < sizeof(TOut),
                   bool>::type
canConvertNumber(TIn) {
  return true;
}
template <typename TOut, typename TIn>
typename enable_if<is_integral<TIn>::value && is_floating_point<TOut>::value,
                   bool>::type
canConvertNumber(TIn) {
  return true;
}
template <typename TOut, typename TIn>
typename enable_if<is_integral<TIn>::value && is_signed<TIn>::value &&
                       is_integral<TOut>::value && is_signed<TOut>::value &&
                       sizeof(TOut) < sizeof(TIn),
                   bool>::type
canConvertNumber(TIn value) {
  return value >= TIn(numeric_limits<TOut>::lowest()) &&
         value <= TIn(numeric_limits<TOut>::highest());
}
template <typename TOut, typename TIn>
typename enable_if<is_integral<TIn>::value && is_signed<TIn>::value &&
                       is_integral<TOut>::value && is_signed<TOut>::value &&
                       sizeof(TIn) <= sizeof(TOut),
                   bool>::type
canConvertNumber(TIn) {
  return true;
}
template <typename TOut, typename TIn>
typename enable_if<is_integral<TIn>::value && is_signed<TIn>::value &&
                       is_integral<TOut>::value && is_unsigned<TOut>::value &&
                       sizeof(TOut) >= sizeof(TIn),
                   bool>::type
canConvertNumber(TIn value) {
  if (value < 0)
    return false;
  return TOut(value) <= numeric_limits<TOut>::highest();
}
template <typename TOut, typename TIn>
typename enable_if<is_integral<TIn>::value && is_signed<TIn>::value &&
                       is_integral<TOut>::value && is_unsigned<TOut>::value &&
                       sizeof(TOut) < sizeof(TIn),
                   bool>::type
canConvertNumber(TIn value) {
  if (value < 0)
    return false;
  return value <= TIn(numeric_limits<TOut>::highest());
}
template <typename TOut, typename TIn>
typename enable_if<is_floating_point<TIn>::value && is_integral<TOut>::value &&
                       sizeof(TOut) < sizeof(TIn),
                   bool>::type
canConvertNumber(TIn value) {
  return value >= numeric_limits<TOut>::lowest() &&
         value <= numeric_limits<TOut>::highest();
}
template <typename TOut, typename TIn>
typename enable_if<is_floating_point<TIn>::value && is_integral<TOut>::value &&
                       sizeof(TOut) >= sizeof(TIn),
                   bool>::type
canConvertNumber(TIn value) {
  return value >= numeric_limits<TOut>::lowest() &&
         value <= FloatTraits<TIn>::template highest_for<TOut>();
}
template <typename TOut, typename TIn>
TOut convertNumber(TIn value) {
  return canConvertNumber<TOut>(value) ? TOut(value) : 0;
}
ARDUINOJSON_END_PRIVATE_NAMESPACE
#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
class VariantData {
  VariantContent content_;  // must be first to allow cast from array to variant
  uint8_t flags_;
 public:
  VariantData() : flags_(VALUE_IS_NULL) {}
  void operator=(const VariantData& src) {
    content_ = src.content_;
    flags_ = uint8_t((flags_ & OWNED_KEY_BIT) | (src.flags_ & ~OWNED_KEY_BIT));
  }
  template <typename TVisitor>
  typename TVisitor::result_type accept(TVisitor& visitor) const {
    switch (type()) {
      case VALUE_IS_FLOAT:
        return visitor.visitFloat(content_.asFloat);
      case VALUE_IS_ARRAY:
        return visitor.visitArray(content_.asCollection);
      case VALUE_IS_OBJECT:
        return visitor.visitObject(content_.asCollection);
      case VALUE_IS_LINKED_STRING:
      case VALUE_IS_OWNED_STRING:
        return visitor.visitString(content_.asString.data,
                                   content_.asString.size);
      case VALUE_IS_OWNED_RAW:
      case VALUE_IS_LINKED_RAW:
        return visitor.visitRawJson(content_.asString.data,
                                    content_.asString.size);
      case VALUE_IS_SIGNED_INTEGER:
        return visitor.visitSignedInteger(content_.asSignedInteger);
      case VALUE_IS_UNSIGNED_INTEGER:
        return visitor.visitUnsignedInteger(content_.asUnsignedInteger);
      case VALUE_IS_BOOLEAN:
        return visitor.visitBoolean(content_.asBoolean != 0);
      default:
        return visitor.visitNull();
    }
  }
  template <typename T>
  T asIntegral() const;
  template <typename T>
  T asFloat() const;
  JsonString asString() const;
  bool asBoolean() const;
  CollectionData* asArray() {
    return isArray() ? &content_.asCollection : 0;
  }
  const CollectionData* asArray() const {
    return const_cast<VariantData*>(this)->asArray();
  }
  const CollectionData* asCollection() const {
    return isCollection() ? &content_.asCollection : 0;
  }
  CollectionData* asObject() {
    return isObject() ? &content_.asCollection : 0;
  }
  const CollectionData* asObject() const {
    return const_cast<VariantData*>(this)->asObject();
  }
  bool copyFrom(const VariantData& src, MemoryPool* pool);
  bool isArray() const {
    return (flags_ & VALUE_IS_ARRAY) != 0;
  }
  bool isBoolean() const {
    return type() == VALUE_IS_BOOLEAN;
  }
  bool isCollection() const {
    return (flags_ & COLLECTION_MASK) != 0;
  }
  template <typename T>
  bool isInteger() const {
    switch (type()) {
      case VALUE_IS_UNSIGNED_INTEGER:
        return canConvertNumber<T>(content_.asUnsignedInteger);
      case VALUE_IS_SIGNED_INTEGER:
        return canConvertNumber<T>(content_.asSignedInteger);
      default:
        return false;
    }
  }
  bool isFloat() const {
    return (flags_ & NUMBER_BIT) != 0;
  }
  bool isString() const {
    return type() == VALUE_IS_LINKED_STRING || type() == VALUE_IS_OWNED_STRING;
  }
  bool isObject() const {
    return (flags_ & VALUE_IS_OBJECT) != 0;
  }
  bool isNull() const {
    return type() == VALUE_IS_NULL;
  }
  bool isEnclosed() const {
    return !isFloat();
  }
  void remove(size_t index) {
    if (isArray())
      content_.asCollection.removeElement(index);
  }
  template <typename TAdaptedString>
  void remove(TAdaptedString key) {
    if (isObject())
      content_.asCollection.removeMember(key);
  }
  void setBoolean(bool value) {
    setType(VALUE_IS_BOOLEAN);
    content_.asBoolean = value;
  }
  void setFloat(JsonFloat value) {
    setType(VALUE_IS_FLOAT);
    content_.asFloat = value;
  }
  void setLinkedRaw(SerializedValue<const char*> value) {
    if (value.data()) {
      setType(VALUE_IS_LINKED_RAW);
      content_.asString.data = value.data();
      content_.asString.size = value.size();
    } else {
      setType(VALUE_IS_NULL);
    }
  }
  template <typename T>
  bool storeOwnedRaw(SerializedValue<T> value, MemoryPool* pool) {
    const char* dup = pool->saveString(adaptString(value.data(), value.size()));
    if (dup) {
      setType(VALUE_IS_OWNED_RAW);
      content_.asString.data = dup;
      content_.asString.size = value.size();
      return true;
    } else {
      setType(VALUE_IS_NULL);
      return false;
    }
  }
  template <typename T>
  typename enable_if<is_unsigned<T>::value>::type setInteger(T value) {
    setType(VALUE_IS_UNSIGNED_INTEGER);
    content_.asUnsignedInteger = static_cast<JsonUInt>(value);
  }
  template <typename T>
  typename enable_if<is_signed<T>::value>::type setInteger(T value) {
    setType(VALUE_IS_SIGNED_INTEGER);
    content_.asSignedInteger = value;
  }
  void setNull() {
    setType(VALUE_IS_NULL);
  }
  void setString(JsonString s) {
    ARDUINOJSON_ASSERT(s);
    if (s.isLinked())
      setType(VALUE_IS_LINKED_STRING);
    else
      setType(VALUE_IS_OWNED_STRING);
    content_.asString.data = s.c_str();
    content_.asString.size = s.size();
  }
  CollectionData& toArray() {
    setType(VALUE_IS_ARRAY);
    content_.asCollection.clear();
    return content_.asCollection;
  }
  CollectionData& toObject() {
    setType(VALUE_IS_OBJECT);
    content_.asCollection.clear();
    return content_.asCollection;
  }
  size_t memoryUsage() const {
    switch (type()) {
      case VALUE_IS_OWNED_STRING:
      case VALUE_IS_OWNED_RAW:
        return content_.asString.size + 1;
      case VALUE_IS_OBJECT:
      case VALUE_IS_ARRAY:
        return content_.asCollection.memoryUsage();
      default:
        return 0;
    }
  }
  size_t size() const {
    return isCollection() ? content_.asCollection.size() : 0;
  }
  VariantData* addElement(MemoryPool* pool) {
    if (isNull())
      toArray();
    if (!isArray())
      return 0;
    return content_.asCollection.addElement(pool);
  }
  VariantData* getElement(size_t index) const {
    const CollectionData* col = asArray();
    return col ? col->getElement(index) : 0;
  }
  VariantData* getOrAddElement(size_t index, MemoryPool* pool) {
    if (isNull())
      toArray();
    if (!isArray())
      return 0;
    return content_.asCollection.getOrAddElement(index, pool);
  }
  template <typename TAdaptedString>
  VariantData* getMember(TAdaptedString key) const {
    const CollectionData* col = asObject();
    return col ? col->getMember(key) : 0;
  }
  template <typename TAdaptedString>
  VariantData* getOrAddMember(TAdaptedString key, MemoryPool* pool) {
    if (isNull())
      toObject();
    if (!isObject())
      return 0;
    return content_.asCollection.getOrAddMember(key, pool);
  }
  void movePointers(ptrdiff_t stringDistance, ptrdiff_t variantDistance) {
    if (flags_ & OWNED_VALUE_BIT)
      content_.asString.data += stringDistance;
    if (flags_ & COLLECTION_MASK)
      content_.asCollection.movePointers(stringDistance, variantDistance);
  }
  uint8_t type() const {
    return flags_ & VALUE_MASK;
  }
  template <typename TAdaptedString>
  inline bool setString(TAdaptedString value, MemoryPool* pool) {
    if (value.isNull()) {
      setNull();
      return true;
    }
    return storeString(pool, value, VariantStringSetter(this));
  }
 private:
  void setType(uint8_t t) {
    flags_ &= OWNED_KEY_BIT;
    flags_ |= t;
  }
  struct VariantStringSetter {
    VariantStringSetter(VariantData* instance) : instance_(instance) {}
    template <typename TStoredString>
    void operator()(TStoredString s) {
      if (s)
        instance_->setString(s);
      else
        instance_->setNull();
    }
    VariantData* instance_;
  };
};
template <typename TResult>
struct Visitor {
  typedef TResult result_type;
  TResult visitArray(const CollectionData&) {
    return TResult();
  }
  TResult visitBoolean(bool) {
    return TResult();
  }
  TResult visitFloat(JsonFloat) {
    return TResult();
  }
  TResult visitSignedInteger(JsonInteger) {
    return TResult();
  }
  TResult visitNull() {
    return TResult();
  }
  TResult visitObject(const CollectionData&) {
    return TResult();
  }
  TResult visitUnsignedInteger(JsonUInt) {
    return TResult();
  }
  TResult visitRawJson(const char*, size_t) {
    return TResult();
  }
  TResult visitString(const char*, size_t) {
    return TResult();
  }
};
template <typename TVisitor>
inline typename TVisitor::result_type variantAccept(const VariantData* var,
                                                    TVisitor& visitor) {
  if (var != 0)
    return var->accept(visitor);
  else
    return visitor.visitNull();
}
inline bool variantCopyFrom(VariantData* dst, const VariantData* src,
                            MemoryPool* pool) {
  if (!dst)
    return false;
  if (!src) {
    dst->setNull();
    return true;
  }
  return dst->copyFrom(*src, pool);
}
inline void variantSetNull(VariantData* var) {
  if (!var)
    return;
  var->setNull();
}
template <typename TAdaptedString>
inline bool variantSetString(VariantData* var, TAdaptedString value,
                             MemoryPool* pool) {
  return var != 0 ? var->setString(value, pool) : 0;
}
inline size_t variantSize(const VariantData* var) {
  return var != 0 ? var->size() : 0;
}
inline CollectionData* variantToArray(VariantData* var) {
  if (!var)
    return 0;
  return &var->toArray();
}
inline CollectionData* variantToObject(VariantData* var) {
  if (!var)
    return 0;
  return &var->toObject();
}
inline VariantData* variantGetElement(const VariantData* var, size_t index) {
  return var != 0 ? var->getElement(index) : 0;
}
inline NO_INLINE VariantData* variantAddElement(VariantData* var,
                                                MemoryPool* pool) {
  return var != 0 ? var->addElement(pool) : 0;
}
inline NO_INLINE VariantData* variantGetOrAddElement(VariantData* var,
                                                     size_t index,
                                                     MemoryPool* pool) {
  return var != 0 ? var->getOrAddElement(index, pool) : 0;
}
template <typename TAdaptedString>
VariantData* variantGetMember(const VariantData* var, TAdaptedString key) {
  if (!var)
    return 0;
  return var->getMember(key);
}
template <typename TAdaptedString>
VariantData* variantGetOrAddMember(VariantData* var, TAdaptedString key,
                                   MemoryPool* pool) {
  if (!var)
    return 0;
  return var->getOrAddMember(key, pool);
}
inline bool variantIsNull(const VariantData* var) {
  return var == 0 || var->isNull();
}
inline size_t variantNesting(const VariantData* var) {
  if (!var)
    return 0;
  const CollectionData* collection = var->asCollection();
  if (!collection)
    return 0;
  size_t maxChildNesting = 0;
  for (const VariantSlot* s = collection->head(); s; s = s->next()) {
    size_t childNesting = variantNesting(s->data());
    if (childNesting > maxChildNesting)
      maxChildNesting = childNesting;
  }
  return maxChildNesting + 1;
}
enum CompareResult {
  COMPARE_RESULT_DIFFER = 0,
  COMPARE_RESULT_EQUAL = 1,
  COMPARE_RESULT_GREATER = 2,
  COMPARE_RESULT_LESS = 4,
  COMPARE_RESULT_GREATER_OR_EQUAL = 3,
  COMPARE_RESULT_LESS_OR_EQUAL = 5
};
template <typename T>
CompareResult arithmeticCompare(const T& lhs, const T& rhs) {
  if (lhs < rhs)
    return COMPARE_RESULT_LESS;
  else if (lhs > rhs)
    return COMPARE_RESULT_GREATER;
  else
    return COMPARE_RESULT_EQUAL;
}
template <typename T1, typename T2>
CompareResult arithmeticCompare(
    const T1& lhs, const T2& rhs,
    typename enable_if<is_integral<T1>::value && is_integral<T2>::value &&
                       sizeof(T1) < sizeof(T2)>::type* = 0) {
  return arithmeticCompare<T2>(static_cast<T2>(lhs), rhs);
}
template <typename T1, typename T2>
CompareResult arithmeticCompare(
    const T1& lhs, const T2& rhs,
    typename enable_if<is_integral<T1>::value && is_integral<T2>::value &&
                       sizeof(T2) < sizeof(T1)>::type* = 0) {
  return arithmeticCompare<T1>(lhs, static_cast<T1>(rhs));
}
template <typename T1, typename T2>
CompareResult arithmeticCompare(
    const T1& lhs, const T2& rhs,
    typename enable_if<is_integral<T1>::value && is_integral<T2>::value &&
                       is_signed<T1>::value == is_signed<T2>::value &&
                       sizeof(T2) == sizeof(T1)>::type* = 0) {
  return arithmeticCompare<T1>(lhs, static_cast<T1>(rhs));
}
template <typename T1, typename T2>
CompareResult arithmeticCompare(
    const T1& lhs, const T2& rhs,
    typename enable_if<is_integral<T1>::value && is_integral<T2>::value &&
                       is_unsigned<T1>::value && is_signed<T2>::value &&
                       sizeof(T2) == sizeof(T1)>::type* = 0) {
  if (rhs < 0)
    return COMPARE_RESULT_GREATER;
  return arithmeticCompare<T1>(lhs, static_cast<T1>(rhs));
}
template <typename T1, typename T2>
CompareResult arithmeticCompare(
    const T1& lhs, const T2& rhs,
    typename enable_if<is_integral<T1>::value && is_integral<T2>::value &&
                       is_signed<T1>::value && is_unsigned<T2>::value &&
                       sizeof(T2) == sizeof(T1)>::type* = 0) {
  if (lhs < 0)
    return COMPARE_RESULT_LESS;
  return arithmeticCompare<T2>(static_cast<T2>(lhs), rhs);
}
template <typename T1, typename T2>
CompareResult arithmeticCompare(
    const T1& lhs, const T2& rhs,
    typename enable_if<is_floating_point<T1>::value ||
                       is_floating_point<T2>::value>::type* = 0) {
  return arithmeticCompare<double>(static_cast<double>(lhs),
                                   static_cast<double>(rhs));
}
template <typename T2>
CompareResult arithmeticCompareNegateLeft(
    JsonUInt, const T2&,
    typename enable_if<is_unsigned<T2>::value>::type* = 0) {
  return COMPARE_RESULT_LESS;
}
template <typename T2>
CompareResult arithmeticCompareNegateLeft(
    JsonUInt lhs, const T2& rhs,
    typename enable_if<is_signed<T2>::value>::type* = 0) {
  if (rhs > 0)
    return COMPARE_RESULT_LESS;
  return arithmeticCompare(-rhs, static_cast<T2>(lhs));
}
template <typename T1>
CompareResult arithmeticCompareNegateRight(
    const T1&, JsonUInt,
    typename enable_if<is_unsigned<T1>::value>::type* = 0) {
  return COMPARE_RESULT_GREATER;
}
template <typename T1>
CompareResult arithmeticCompareNegateRight(
    const T1& lhs, JsonUInt rhs,
    typename enable_if<is_signed<T1>::value>::type* = 0) {
  if (lhs > 0)
    return COMPARE_RESULT_GREATER;
  return arithmeticCompare(static_cast<T1>(rhs), -lhs);
}
struct VariantTag {};
template <typename T>
struct IsVariant : is_base_of<VariantTag, T> {};
ARDUINOJSON_END_PRIVATE_NAMESPACE
ARDUINOJSON_BEGIN_PUBLIC_NAMESPACE
class JsonVariantConst;
ARDUINOJSON_END_PUBLIC_NAMESPACE
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <typename T>
CompareResult compare(JsonVariantConst lhs,
                      const T& rhs);  // VariantCompare.cpp
struct VariantOperatorTag {};
template <typename TVariant>
struct VariantOperators : VariantOperatorTag {
  template <typename T>
  friend
      typename enable_if<!IsVariant<T>::value && !is_array<T>::value, T>::type
      operator|(const TVariant& variant, const T& defaultValue) {
    if (variant.template is<T>())
      return variant.template as<T>();
    else
      return defaultValue;
  }
  friend const char* operator|(const TVariant& variant,
                               const char* defaultValue) {
    if (variant.template is<const char*>())
      return variant.template as<const char*>();
    else
      return defaultValue;
  }
  template <typename T>
  friend typename enable_if<IsVariant<T>::value, JsonVariantConst>::type
  operator|(const TVariant& variant, T defaultValue) {
    if (variant)
      return variant;
    else
      return defaultValue;
  }
  template <typename T>
  friend bool operator==(T* lhs, TVariant rhs) {
    return compare(rhs, lhs) == COMPARE_RESULT_EQUAL;
  }
  template <typename T>
  friend bool operator==(const T& lhs, TVariant rhs) {
    return compare(rhs, lhs) == COMPARE_RESULT_EQUAL;
  }
  template <typename T>
  friend bool operator==(TVariant lhs, T* rhs) {
    return compare(lhs, rhs) == COMPARE_RESULT_EQUAL;
  }
  template <typename T>
  friend
      typename enable_if<!is_base_of<VariantOperatorTag, T>::value, bool>::type
      operator==(TVariant lhs, const T& rhs) {
    return compare(lhs, rhs) == COMPARE_RESULT_EQUAL;
  }
  template <typename T>
  friend bool operator!=(T* lhs, TVariant rhs) {
    return compare(rhs, lhs) != COMPARE_RESULT_EQUAL;
  }
  template <typename T>
  friend bool operator!=(const T& lhs, TVariant rhs) {
    return compare(rhs, lhs) != COMPARE_RESULT_EQUAL;
  }
  template <typename T>
  friend bool operator!=(TVariant lhs, T* rhs) {
    return compare(lhs, rhs) != COMPARE_RESULT_EQUAL;
  }
  template <typename T>
  friend
      typename enable_if<!is_base_of<VariantOperatorTag, T>::value, bool>::type
      operator!=(TVariant lhs, const T& rhs) {
    return compare(lhs, rhs) != COMPARE_RESULT_EQUAL;
  }
  template <typename T>
  friend bool operator<(T* lhs, TVariant rhs) {
    return compare(rhs, lhs) == COMPARE_RESULT_GREATER;
  }
  template <typename T>
  friend bool operator<(const T& lhs, TVariant rhs) {
    return compare(rhs, lhs) == COMPARE_RESULT_GREATER;
  }
  template <typename T>
  friend bool operator<(TVariant lhs, T* rhs) {
    return compare(lhs, rhs) == COMPARE_RESULT_LESS;
  }
  template <typename T>
  friend
      typename enable_if<!is_base_of<VariantOperatorTag, T>::value, bool>::type
      operator<(TVariant lhs, const T& rhs) {
    return compare(lhs, rhs) == COMPARE_RESULT_LESS;
  }
  template <typename T>
  friend bool operator<=(T* lhs, TVariant rhs) {
    return (compare(rhs, lhs) & COMPARE_RESULT_GREATER_OR_EQUAL) != 0;
  }
  template <typename T>
  friend bool operator<=(const T& lhs, TVariant rhs) {
    return (compare(rhs, lhs) & COMPARE_RESULT_GREATER_OR_EQUAL) != 0;
  }
  template <typename T>
  friend bool operator<=(TVariant lhs, T* rhs) {
    return (compare(lhs, rhs) & COMPARE_RESULT_LESS_OR_EQUAL) != 0;
  }
  template <typename T>
  friend
      typename enable_if<!is_base_of<VariantOperatorTag, T>::value, bool>::type
      operator<=(TVariant lhs, const T& rhs) {
    return (compare(lhs, rhs) & COMPARE_RESULT_LESS_OR_EQUAL) != 0;
  }
  template <typename T>
  friend bool operator>(T* lhs, TVariant rhs) {
    return compare(rhs, lhs) == COMPARE_RESULT_LESS;
  }
  template <typename T>
  friend bool operator>(const T& lhs, TVariant rhs) {
    return compare(rhs, lhs) == COMPARE_RESULT_LESS;
  }
  template <typename T>
  friend bool operator>(TVariant lhs, T* rhs) {
    return compare(lhs, rhs) == COMPARE_RESULT_GREATER;
  }
  template <typename T>
  friend
      typename enable_if<!is_base_of<VariantOperatorTag, T>::value, bool>::type
      operator>(TVariant lhs, const T& rhs) {
    return compare(lhs, rhs) == COMPARE_RESULT_GREATER;
  }
  template <typename T>
  friend bool operator>=(T* lhs, TVariant rhs) {
    return (compare(rhs, lhs) & COMPARE_RESULT_LESS_OR_EQUAL) != 0;
  }
  template <typename T>
  friend bool operator>=(const T& lhs, TVariant rhs) {
    return (compare(rhs, lhs) & COMPARE_RESULT_LESS_OR_EQUAL) != 0;
  }
  template <typename T>
  friend bool operator>=(TVariant lhs, T* rhs) {
    return (compare(lhs, rhs) & COMPARE_RESULT_GREATER_OR_EQUAL) != 0;
  }
  template <typename T>
  friend
      typename enable_if<!is_base_of<VariantOperatorTag, T>::value, bool>::type
      operator>=(TVariant lhs, const T& rhs) {
    return (compare(lhs, rhs) & COMPARE_RESULT_GREATER_OR_EQUAL) != 0;
  }
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
ARDUINOJSON_BEGIN_PUBLIC_NAMESPACE
class JsonArray;
class JsonObject;
class JsonVariantConst : public detail::VariantTag,
                         public detail::VariantOperators<JsonVariantConst> {
  friend class detail::VariantAttorney;
 public:
  JsonVariantConst() : data_(0) {}
  explicit JsonVariantConst(const detail::VariantData* data) : data_(data) {}
  FORCE_INLINE bool isNull() const {
    using namespace detail;
    return variantIsNull(data_);
  }
  FORCE_INLINE bool isUnbound() const {
    return !data_;
  }
  FORCE_INLINE size_t memoryUsage() const {
    return data_ ? data_->memoryUsage() : 0;
  }
  FORCE_INLINE size_t nesting() const {
    return variantNesting(data_);
  }
  size_t size() const {
    return variantSize(data_);
  }
  template <typename T>
  FORCE_INLINE typename detail::enable_if<!detail::is_same<T, char*>::value &&
                                              !detail::is_same<T, char>::value,
                                          T>::type
  as() const {
    return Converter<T>::fromJson(*this);
  }
  template <typename T>
  FORCE_INLINE typename detail::enable_if<!detail::is_same<T, char*>::value &&
                                              !detail::is_same<T, char>::value,
                                          bool>::type
  is() const {
    return Converter<T>::checkJson(*this);
  }
  template <typename T>
  FORCE_INLINE operator T() const {
    return as<T>();
  }
  FORCE_INLINE JsonVariantConst operator[](size_t index) const {
    return JsonVariantConst(variantGetElement(data_, index));
  }
  template <typename TString>
  FORCE_INLINE typename detail::enable_if<detail::IsString<TString>::value,
                                          JsonVariantConst>::type
  operator[](const TString& key) const {
    return JsonVariantConst(variantGetMember(data_, detail::adaptString(key)));
  }
  template <typename TChar>
  FORCE_INLINE typename detail::enable_if<detail::IsString<TChar*>::value,
                                          JsonVariantConst>::type
  operator[](TChar* key) const {
    return JsonVariantConst(variantGetMember(data_, detail::adaptString(key)));
  }
  template <typename TString>
  FORCE_INLINE
      typename detail::enable_if<detail::IsString<TString>::value, bool>::type
      containsKey(const TString& key) const {
    return variantGetMember(getData(), detail::adaptString(key)) != 0;
  }
  template <typename TChar>
  FORCE_INLINE
      typename detail::enable_if<detail::IsString<TChar*>::value, bool>::type
      containsKey(TChar* key) const {
    return variantGetMember(getData(), detail::adaptString(key)) != 0;
  }
 protected:
  const detail::VariantData* getData() const {
    return data_;
  }
 private:
  const detail::VariantData* data_;
};
class JsonVariant;
ARDUINOJSON_END_PUBLIC_NAMESPACE
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <typename>
class ElementProxy;
template <typename, typename>
class MemberProxy;
template <typename TDerived>
class VariantRefBase : public VariantTag {
  friend class VariantAttorney;
 public:
  FORCE_INLINE void clear() const {
    variantSetNull(getData());
  }
  FORCE_INLINE bool isNull() const {
    return variantIsNull(getData());
  }
  FORCE_INLINE bool isUnbound() const {
    return !getData();
  }
  template <typename T>
  FORCE_INLINE
      typename enable_if<!ConverterNeedsWriteableRef<T>::value, T>::type
      as() const {
    return Converter<T>::fromJson(getVariantConst());
  }
  template <typename T>
  FORCE_INLINE typename enable_if<ConverterNeedsWriteableRef<T>::value, T>::type
  as() const;
  template <typename T,
            typename = typename enable_if<!is_same<T, TDerived>::value>::type>
  FORCE_INLINE operator T() const {
    return as<T>();
  }
  template <typename T>
  typename enable_if<is_same<T, JsonArray>::value, JsonArray>::type to() const;
  template <typename T>
  typename enable_if<is_same<T, JsonObject>::value, JsonObject>::type to()
      const;
  template <typename T>
  typename enable_if<is_same<T, JsonVariant>::value, JsonVariant>::type to()
      const;
  template <typename T>
  FORCE_INLINE
      typename enable_if<ConverterNeedsWriteableRef<T>::value, bool>::type
      is() const;
  template <typename T>
  FORCE_INLINE typename enable_if<!ConverterNeedsWriteableRef<T>::value &&
                                      !is_same<T, char*>::value &&
                                      !is_same<T, char>::value,
                                  bool>::type
  is() const {
    return Converter<T>::checkJson(getVariantConst());
  }
  FORCE_INLINE void shallowCopy(ArduinoJson::JsonVariantConst target) {
    VariantData* data = getOrCreateData();
    if (!data)
      return;
    const VariantData* targetData = VariantAttorney::getData(target);
    if (targetData)
      *data = *targetData;
    else
      data->setNull();
  }
  template <typename T>
  FORCE_INLINE bool set(const T& value) const;
  template <typename T>
  FORCE_INLINE bool set(T* value) const;
  FORCE_INLINE size_t size() const {
    return variantSize(getData());
  }
  FORCE_INLINE size_t memoryUsage() const {
    VariantData* data = getData();
    return data ? data->memoryUsage() : 0;
  }
  FORCE_INLINE size_t nesting() const {
    return variantNesting(getData());
  }
  FORCE_INLINE JsonVariant add() const;
  template <typename T>
  FORCE_INLINE bool add(const T& value) const {
    return add().set(value);
  }
  template <typename T>
  FORCE_INLINE bool add(T* value) const {
    return add().set(value);
  }
  FORCE_INLINE void remove(size_t index) const {
    VariantData* data = getData();
    if (data)
      data->remove(index);
  }
  template <typename TChar>
  FORCE_INLINE typename enable_if<IsString<TChar*>::value>::type remove(
      TChar* key) const {
    VariantData* data = getData();
    if (data)
      data->remove(adaptString(key));
  }
  template <typename TString>
  FORCE_INLINE typename enable_if<IsString<TString>::value>::type remove(
      const TString& key) const {
    VariantData* data = getData();
    if (data)
      data->remove(adaptString(key));
  }
  FORCE_INLINE JsonArray createNestedArray() const;
  FORCE_INLINE JsonObject createNestedObject() const;
  FORCE_INLINE ElementProxy<TDerived> operator[](size_t index) const;
  template <typename TString>
  FORCE_INLINE typename enable_if<IsString<TString>::value, bool>::type
  containsKey(const TString& key) const;
  template <typename TChar>
  FORCE_INLINE typename enable_if<IsString<TChar*>::value, bool>::type
  containsKey(TChar* key) const;
  template <typename TString>
  FORCE_INLINE typename enable_if<IsString<TString>::value,
                                  MemberProxy<TDerived, TString>>::type
  operator[](const TString& key) const;
  template <typename TChar>
  FORCE_INLINE typename enable_if<IsString<TChar*>::value,
                                  MemberProxy<TDerived, TChar*>>::type
  operator[](TChar* key) const;
  template <typename TString>
  FORCE_INLINE JsonArray createNestedArray(const TString& key) const;
  template <typename TChar>
  FORCE_INLINE JsonArray createNestedArray(TChar* key) const;
  template <typename TString>
  JsonObject createNestedObject(const TString& key) const;
  template <typename TChar>
  JsonObject createNestedObject(TChar* key) const;
 private:
  TDerived& derived() {
    return static_cast<TDerived&>(*this);
  }
  const TDerived& derived() const {
    return static_cast<const TDerived&>(*this);
  }
  FORCE_INLINE MemoryPool* getPool() const {
    return VariantAttorney::getPool(derived());
  }
  FORCE_INLINE VariantData* getData() const {
    return VariantAttorney::getData(derived());
  }
  FORCE_INLINE VariantData* getOrCreateData() const {
    return VariantAttorney::getOrCreateData(derived());
  }
 private:
  FORCE_INLINE ArduinoJson::JsonVariant getVariant() const;
  FORCE_INLINE ArduinoJson::JsonVariantConst getVariantConst() const {
    return ArduinoJson::JsonVariantConst(getData());
  }
  FORCE_INLINE ArduinoJson::JsonVariant getOrCreateVariant() const;
};
template <typename TUpstream>
class ElementProxy : public VariantRefBase<ElementProxy<TUpstream>>,
                     public VariantOperators<ElementProxy<TUpstream>> {
  friend class VariantAttorney;
 public:
  ElementProxy(TUpstream upstream, size_t index)
      : upstream_(upstream), index_(index) {}
  ElementProxy(const ElementProxy& src)
      : upstream_(src.upstream_), index_(src.index_) {}
  FORCE_INLINE ElementProxy& operator=(const ElementProxy& src) {
    this->set(src);
    return *this;
  }
  template <typename T>
  FORCE_INLINE ElementProxy& operator=(const T& src) {
    this->set(src);
    return *this;
  }
  template <typename T>
  FORCE_INLINE ElementProxy& operator=(T* src) {
    this->set(src);
    return *this;
  }
 private:
  FORCE_INLINE MemoryPool* getPool() const {
    return VariantAttorney::getPool(upstream_);
  }
  FORCE_INLINE VariantData* getData() const {
    return variantGetElement(VariantAttorney::getData(upstream_), index_);
  }
  FORCE_INLINE VariantData* getOrCreateData() const {
    return variantGetOrAddElement(VariantAttorney::getOrCreateData(upstream_),
                                  index_, VariantAttorney::getPool(upstream_));
  }
  TUpstream upstream_;
  size_t index_;
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
ARDUINOJSON_BEGIN_PUBLIC_NAMESPACE
class JsonVariant : public detail::VariantRefBase<JsonVariant>,
                    public detail::VariantOperators<JsonVariant> {
  friend class detail::VariantAttorney;
 public:
  JsonVariant() : data_(0), pool_(0) {}
  JsonVariant(detail::MemoryPool* pool, detail::VariantData* data)
      : data_(data), pool_(pool) {}
 private:
  FORCE_INLINE detail::MemoryPool* getPool() const {
    return pool_;
  }
  FORCE_INLINE detail::VariantData* getData() const {
    return data_;
  }
  FORCE_INLINE detail::VariantData* getOrCreateData() const {
    return data_;
  }
  detail::VariantData* data_;
  detail::MemoryPool* pool_;
};
template <>
struct Converter<JsonVariant> : private detail::VariantAttorney {
  static void toJson(JsonVariant src, JsonVariant dst) {
    detail::variantCopyFrom(getData(dst), getData(src), getPool(dst));
  }
  static JsonVariant fromJson(JsonVariant src) {
    return src;
  }
  static detail::InvalidConversion<JsonVariantConst, JsonVariant> fromJson(
      JsonVariantConst);
  static bool checkJson(JsonVariant src) {
    auto data = getData(src);
    return !!data;
  }
  static bool checkJson(JsonVariantConst) {
    return false;
  }
};
template <>
struct Converter<JsonVariantConst> : private detail::VariantAttorney {
  static void toJson(JsonVariantConst src, JsonVariant dst) {
    variantCopyFrom(getData(dst), getData(src), getPool(dst));
  }
  static JsonVariantConst fromJson(JsonVariantConst src) {
    return JsonVariantConst(getData(src));
  }
  static bool checkJson(JsonVariantConst src) {
    auto data = getData(src);
    return !!data;
  }
};
ARDUINOJSON_END_PUBLIC_NAMESPACE
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
struct SlotKeySetter {
  SlotKeySetter(VariantSlot* instance) : instance_(instance) {}
  template <typename TStoredString>
  void operator()(TStoredString s) {
    if (!s)
      return;
    ARDUINOJSON_ASSERT(instance_ != 0);
    instance_->setKey(s);
  }
  VariantSlot* instance_;
};
template <typename TAdaptedString>
inline bool slotSetKey(VariantSlot* var, TAdaptedString key, MemoryPool* pool) {
  if (!var)
    return false;
  return storeString(pool, key, SlotKeySetter(var));
}
inline size_t slotSize(const VariantSlot* var) {
  size_t n = 0;
  while (var) {
    n++;
    var = var->next();
  }
  return n;
}
inline VariantData* slotData(VariantSlot* slot) {
  return reinterpret_cast<VariantData*>(slot);
}
ARDUINOJSON_END_PRIVATE_NAMESPACE
ARDUINOJSON_BEGIN_PUBLIC_NAMESPACE
class VariantPtr {
 public:
  VariantPtr(detail::MemoryPool* pool, detail::VariantData* data)
      : variant_(pool, data) {}
  JsonVariant* operator->() {
    return &variant_;
  }
  JsonVariant& operator*() {
    return variant_;
  }
 private:
  JsonVariant variant_;
};
class JsonArrayIterator {
  friend class JsonArray;
 public:
  JsonArrayIterator() : slot_(0) {}
  explicit JsonArrayIterator(detail::MemoryPool* pool,
                             detail::VariantSlot* slot)
      : pool_(pool), slot_(slot) {}
  JsonVariant operator*() const {
    return JsonVariant(pool_, slot_->data());
  }
  VariantPtr operator->() {
    return VariantPtr(pool_, slot_->data());
  }
  bool operator==(const JsonArrayIterator& other) const {
    return slot_ == other.slot_;
  }
  bool operator!=(const JsonArrayIterator& other) const {
    return slot_ != other.slot_;
  }
  JsonArrayIterator& operator++() {
    slot_ = slot_->next();
    return *this;
  }
  JsonArrayIterator& operator+=(size_t distance) {
    slot_ = slot_->next(distance);
    return *this;
  }
 private:
  detail::MemoryPool* pool_;
  detail::VariantSlot* slot_;
};
class VariantConstPtr {
 public:
  VariantConstPtr(const detail::VariantData* data) : variant_(data) {}
  JsonVariantConst* operator->() {
    return &variant_;
  }
  JsonVariantConst& operator*() {
    return variant_;
  }
 private:
  JsonVariantConst variant_;
};
class JsonArrayConstIterator {
  friend class JsonArray;
 public:
  JsonArrayConstIterator() : slot_(0) {}
  explicit JsonArrayConstIterator(const detail::VariantSlot* slot)
      : slot_(slot) {}
  JsonVariantConst operator*() const {
    return JsonVariantConst(slot_->data());
  }
  VariantConstPtr operator->() {
    return VariantConstPtr(slot_->data());
  }
  bool operator==(const JsonArrayConstIterator& other) const {
    return slot_ == other.slot_;
  }
  bool operator!=(const JsonArrayConstIterator& other) const {
    return slot_ != other.slot_;
  }
  JsonArrayConstIterator& operator++() {
    slot_ = slot_->next();
    return *this;
  }
  JsonArrayConstIterator& operator+=(size_t distance) {
    slot_ = slot_->next(distance);
    return *this;
  }
 private:
  const detail::VariantSlot* slot_;
};
class JsonObject;
class JsonArrayConst : public detail::VariantOperators<JsonArrayConst> {
  friend class JsonArray;
  friend class detail::VariantAttorney;
 public:
  typedef JsonArrayConstIterator iterator;
  FORCE_INLINE iterator begin() const {
    if (!data_)
      return iterator();
    return iterator(data_->head());
  }
  FORCE_INLINE iterator end() const {
    return iterator();
  }
  FORCE_INLINE JsonArrayConst() : data_(0) {}
  FORCE_INLINE JsonArrayConst(const detail::CollectionData* data)
      : data_(data) {}
  FORCE_INLINE bool operator==(JsonArrayConst rhs) const {
    if (data_ == rhs.data_)
      return true;
    if (!data_ || !rhs.data_)
      return false;
    iterator it1 = begin();
    iterator it2 = rhs.begin();
    for (;;) {
      bool end1 = it1 == end();
      bool end2 = it2 == rhs.end();
      if (end1 && end2)
        return true;
      if (end1 || end2)
        return false;
      if (*it1 != *it2)
        return false;
      ++it1;
      ++it2;
    }
  }
  FORCE_INLINE JsonVariantConst operator[](size_t index) const {
    return JsonVariantConst(data_ ? data_->getElement(index) : 0);
  }
  operator JsonVariantConst() const {
    return JsonVariantConst(collectionToVariant(data_));
  }
  FORCE_INLINE bool isNull() const {
    return data_ == 0;
  }
  FORCE_INLINE operator bool() const {
    return data_ != 0;
  }
  FORCE_INLINE size_t memoryUsage() const {
    return data_ ? data_->memoryUsage() : 0;
  }
  FORCE_INLINE size_t nesting() const {
    return variantNesting(collectionToVariant(data_));
  }
  FORCE_INLINE size_t size() const {
    return data_ ? data_->size() : 0;
  }
 private:
  const detail::VariantData* getData() const {
    return collectionToVariant(data_);
  }
  const detail::CollectionData* data_;
};
template <>
struct Converter<JsonArrayConst> : private detail::VariantAttorney {
  static void toJson(JsonVariantConst src, JsonVariant dst) {
    variantCopyFrom(getData(dst), getData(src), getPool(dst));
  }
  static JsonArrayConst fromJson(JsonVariantConst src) {
    auto data = getData(src);
    return data ? data->asArray() : 0;
  }
  static bool checkJson(JsonVariantConst src) {
    auto data = getData(src);
    return data && data->isArray();
  }
};
class JsonObject;
class JsonArray : public detail::VariantOperators<JsonArray> {
  friend class detail::VariantAttorney;
 public:
  typedef JsonArrayIterator iterator;
  FORCE_INLINE JsonArray() : data_(0), pool_(0) {}
  FORCE_INLINE JsonArray(detail::MemoryPool* pool, detail::CollectionData* data)
      : data_(data), pool_(pool) {}
  operator JsonVariant() {
    void* data = data_;  // prevent warning cast-align
    return JsonVariant(pool_, reinterpret_cast<detail::VariantData*>(data));
  }
  operator JsonArrayConst() const {
    return JsonArrayConst(data_);
  }
  JsonVariant add() const {
    if (!data_)
      return JsonVariant();
    return JsonVariant(pool_, data_->addElement(pool_));
  }
  template <typename T>
  FORCE_INLINE bool add(const T& value) const {
    return add().set(value);
  }
  template <typename T>
  FORCE_INLINE bool add(T* value) const {
    return add().set(value);
  }
  FORCE_INLINE iterator begin() const {
    if (!data_)
      return iterator();
    return iterator(pool_, data_->head());
  }
  FORCE_INLINE iterator end() const {
    return iterator();
  }
  FORCE_INLINE bool set(JsonArrayConst src) const {
    if (!data_ || !src.data_)
      return false;
    return data_->copyFrom(*src.data_, pool_);
  }
  FORCE_INLINE bool operator==(JsonArray rhs) const {
    return JsonArrayConst(data_) == JsonArrayConst(rhs.data_);
  }
  FORCE_INLINE void remove(iterator it) const {
    if (!data_)
      return;
    data_->removeSlot(it.slot_);
  }
  FORCE_INLINE void remove(size_t index) const {
    if (!data_)
      return;
    data_->removeElement(index);
  }
  void clear() const {
    if (!data_)
      return;
    data_->clear();
  }
  FORCE_INLINE detail::ElementProxy<JsonArray> operator[](size_t index) const {
    return {*this, index};
  }
  FORCE_INLINE JsonObject createNestedObject() const;
  FORCE_INLINE JsonArray createNestedArray() const {
    return add().to<JsonArray>();
  }
  operator JsonVariantConst() const {
    return JsonVariantConst(collectionToVariant(data_));
  }
  FORCE_INLINE bool isNull() const {
    return data_ == 0;
  }
  FORCE_INLINE operator bool() const {
    return data_ != 0;
  }
  FORCE_INLINE size_t memoryUsage() const {
    return data_ ? data_->memoryUsage() : 0;
  }
  FORCE_INLINE size_t nesting() const {
    return variantNesting(collectionToVariant(data_));
  }
  FORCE_INLINE size_t size() const {
    return data_ ? data_->size() : 0;
  }
 private:
  detail::MemoryPool* getPool() const {
    return pool_;
  }
  detail::VariantData* getData() const {
    return collectionToVariant(data_);
  }
  detail::VariantData* getOrCreateData() const {
    return collectionToVariant(data_);
  }
  detail::CollectionData* data_;
  detail::MemoryPool* pool_;
};
template <>
struct Converter<JsonArray> : private detail::VariantAttorney {
  static void toJson(JsonVariantConst src, JsonVariant dst) {
    variantCopyFrom(getData(dst), getData(src), getPool(dst));
  }
  static JsonArray fromJson(JsonVariant src) {
    auto data = getData(src);
    auto pool = getPool(src);
    return JsonArray(pool, data != 0 ? data->asArray() : 0);
  }
  static detail::InvalidConversion<JsonVariantConst, JsonArray> fromJson(
      JsonVariantConst);
  static bool checkJson(JsonVariantConst) {
    return false;
  }
  static bool checkJson(JsonVariant src) {
    auto data = getData(src);
    return data && data->isArray();
  }
};
class JsonPair {
 public:
  JsonPair(detail::MemoryPool* pool, detail::VariantSlot* slot) {
    if (slot) {
      key_ = JsonString(slot->key(), slot->ownsKey() ? JsonString::Copied
                                                     : JsonString::Linked);
      value_ = JsonVariant(pool, slot->data());
    }
  }
  JsonString key() const {
    return key_;
  }
  JsonVariant value() const {
    return value_;
  }
 private:
  JsonString key_;
  JsonVariant value_;
};
class JsonPairConst {
 public:
  JsonPairConst(const detail::VariantSlot* slot) {
    if (slot) {
      key_ = JsonString(slot->key(), slot->ownsKey() ? JsonString::Copied
                                                     : JsonString::Linked);
      value_ = JsonVariantConst(slot->data());
    }
  }
  JsonString key() const {
    return key_;
  }
  JsonVariantConst value() const {
    return value_;
  }
 private:
  JsonString key_;
  JsonVariantConst value_;
};
class JsonPairPtr {
 public:
  JsonPairPtr(detail::MemoryPool* pool, detail::VariantSlot* slot)
      : pair_(pool, slot) {}
  const JsonPair* operator->() const {
    return &pair_;
  }
  const JsonPair& operator*() const {
    return pair_;
  }
 private:
  JsonPair pair_;
};
class JsonObjectIterator {
  friend class JsonObject;
 public:
  JsonObjectIterator() : slot_(0) {}
  explicit JsonObjectIterator(detail::MemoryPool* pool,
                              detail::VariantSlot* slot)
      : pool_(pool), slot_(slot) {}
  JsonPair operator*() const {
    return JsonPair(pool_, slot_);
  }
  JsonPairPtr operator->() {
    return JsonPairPtr(pool_, slot_);
  }
  bool operator==(const JsonObjectIterator& other) const {
    return slot_ == other.slot_;
  }
  bool operator!=(const JsonObjectIterator& other) const {
    return slot_ != other.slot_;
  }
  JsonObjectIterator& operator++() {
    slot_ = slot_->next();
    return *this;
  }
  JsonObjectIterator& operator+=(size_t distance) {
    slot_ = slot_->next(distance);
    return *this;
  }
 private:
  detail::MemoryPool* pool_;
  detail::VariantSlot* slot_;
};
class JsonPairConstPtr {
 public:
  JsonPairConstPtr(const detail::VariantSlot* slot) : pair_(slot) {}
  const JsonPairConst* operator->() const {
    return &pair_;
  }
  const JsonPairConst& operator*() const {
    return pair_;
  }
 private:
  JsonPairConst pair_;
};
class JsonObjectConstIterator {
  friend class JsonObject;
 public:
  JsonObjectConstIterator() : slot_(0) {}
  explicit JsonObjectConstIterator(const detail::VariantSlot* slot)
      : slot_(slot) {}
  JsonPairConst operator*() const {
    return JsonPairConst(slot_);
  }
  JsonPairConstPtr operator->() {
    return JsonPairConstPtr(slot_);
  }
  bool operator==(const JsonObjectConstIterator& other) const {
    return slot_ == other.slot_;
  }
  bool operator!=(const JsonObjectConstIterator& other) const {
    return slot_ != other.slot_;
  }
  JsonObjectConstIterator& operator++() {
    slot_ = slot_->next();
    return *this;
  }
  JsonObjectConstIterator& operator+=(size_t distance) {
    slot_ = slot_->next(distance);
    return *this;
  }
 private:
  const detail::VariantSlot* slot_;
};
class JsonObjectConst : public detail::VariantOperators<JsonObjectConst> {
  friend class JsonObject;
  friend class detail::VariantAttorney;
 public:
  typedef JsonObjectConstIterator iterator;
  JsonObjectConst() : data_(0) {}
  JsonObjectConst(const detail::CollectionData* data) : data_(data) {}
  operator JsonVariantConst() const {
    return JsonVariantConst(collectionToVariant(data_));
  }
  FORCE_INLINE bool isNull() const {
    return data_ == 0;
  }
  FORCE_INLINE operator bool() const {
    return data_ != 0;
  }
  FORCE_INLINE size_t memoryUsage() const {
    return data_ ? data_->memoryUsage() : 0;
  }
  FORCE_INLINE size_t nesting() const {
    return variantNesting(collectionToVariant(data_));
  }
  FORCE_INLINE size_t size() const {
    return data_ ? data_->size() : 0;
  }
  FORCE_INLINE iterator begin() const {
    if (!data_)
      return iterator();
    return iterator(data_->head());
  }
  FORCE_INLINE iterator end() const {
    return iterator();
  }
  template <typename TString>
  FORCE_INLINE bool containsKey(const TString& key) const {
    return getMember(detail::adaptString(key)) != 0;
  }
  template <typename TChar>
  FORCE_INLINE bool containsKey(TChar* key) const {
    return getMember(detail::adaptString(key)) != 0;
  }
  template <typename TString>
  FORCE_INLINE typename detail::enable_if<detail::IsString<TString>::value,
                                          JsonVariantConst>::type
  operator[](const TString& key) const {
    return JsonVariantConst(getMember(detail::adaptString(key)));
  }
  template <typename TChar>
  FORCE_INLINE typename detail::enable_if<detail::IsString<TChar*>::value,
                                          JsonVariantConst>::type
  operator[](TChar* key) const {
    return JsonVariantConst(getMember(detail::adaptString(key)));
  }
  FORCE_INLINE bool operator==(JsonObjectConst rhs) const {
    if (data_ == rhs.data_)
      return true;
    if (!data_ || !rhs.data_)
      return false;
    size_t count = 0;
    for (iterator it = begin(); it != end(); ++it) {
      if (it->value() != rhs[it->key()])
        return false;
      count++;
    }
    return count == rhs.size();
  }
 private:
  const detail::VariantData* getData() const {
    return collectionToVariant(data_);
  }
  template <typename TAdaptedString>
  const detail::VariantData* getMember(TAdaptedString key) const {
    if (!data_)
      return 0;
    return data_->getMember(key);
  }
  const detail::CollectionData* data_;
};
template <>
struct Converter<JsonObjectConst> : private detail::VariantAttorney {
  static void toJson(JsonVariantConst src, JsonVariant dst) {
    variantCopyFrom(getData(dst), getData(src), getPool(dst));
  }
  static JsonObjectConst fromJson(JsonVariantConst src) {
    auto data = getData(src);
    return data != 0 ? data->asObject() : 0;
  }
  static bool checkJson(JsonVariantConst src) {
    auto data = getData(src);
    return data && data->isObject();
  }
};
ARDUINOJSON_END_PUBLIC_NAMESPACE
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <typename TUpstream, typename TStringRef>
class MemberProxy
    : public VariantRefBase<MemberProxy<TUpstream, TStringRef>>,
      public VariantOperators<MemberProxy<TUpstream, TStringRef>> {
  friend class VariantAttorney;
 public:
  FORCE_INLINE MemberProxy(TUpstream upstream, TStringRef key)
      : upstream_(upstream), key_(key) {}
  MemberProxy(const MemberProxy& src)
      : upstream_(src.upstream_), key_(src.key_) {}
  FORCE_INLINE MemberProxy& operator=(const MemberProxy& src) {
    this->set(src);
    return *this;
  }
  template <typename T>
  FORCE_INLINE MemberProxy& operator=(const T& src) {
    this->set(src);
    return *this;
  }
  template <typename T>
  FORCE_INLINE MemberProxy& operator=(T* src) {
    this->set(src);
    return *this;
  }
 private:
  FORCE_INLINE MemoryPool* getPool() const {
    return VariantAttorney::getPool(upstream_);
  }
  FORCE_INLINE VariantData* getData() const {
    return variantGetMember(VariantAttorney::getData(upstream_),
                            adaptString(key_));
  }
  FORCE_INLINE VariantData* getOrCreateData() const {
    return variantGetOrAddMember(VariantAttorney::getOrCreateData(upstream_),
                                 adaptString(key_),
                                 VariantAttorney::getPool(upstream_));
  }
 private:
  TUpstream upstream_;
  TStringRef key_;
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
ARDUINOJSON_BEGIN_PUBLIC_NAMESPACE
class JsonArray;
class JsonObject : public detail::VariantOperators<JsonObject> {
  friend class detail::VariantAttorney;
 public:
  typedef JsonObjectIterator iterator;
  FORCE_INLINE JsonObject() : data_(0), pool_(0) {}
  FORCE_INLINE JsonObject(detail::MemoryPool* buf, detail::CollectionData* data)
      : data_(data), pool_(buf) {}
  operator JsonVariant() const {
    void* data = data_;  // prevent warning cast-align
    return JsonVariant(pool_, reinterpret_cast<detail::VariantData*>(data));
  }
  operator JsonObjectConst() const {
    return JsonObjectConst(data_);
  }
  operator JsonVariantConst() const {
    return JsonVariantConst(collectionToVariant(data_));
  }
  FORCE_INLINE bool isNull() const {
    return data_ == 0;
  }
  FORCE_INLINE operator bool() const {
    return data_ != 0;
  }
  FORCE_INLINE size_t memoryUsage() const {
    return data_ ? data_->memoryUsage() : 0;
  }
  FORCE_INLINE size_t nesting() const {
    return variantNesting(collectionToVariant(data_));
  }
  FORCE_INLINE size_t size() const {
    return data_ ? data_->size() : 0;
  }
  FORCE_INLINE iterator begin() const {
    if (!data_)
      return iterator();
    return iterator(pool_, data_->head());
  }
  FORCE_INLINE iterator end() const {
    return iterator();
  }
  void clear() const {
    if (!data_)
      return;
    data_->clear();
  }
  FORCE_INLINE bool set(JsonObjectConst src) {
    if (!data_ || !src.data_)
      return false;
    return data_->copyFrom(*src.data_, pool_);
  }
  FORCE_INLINE bool operator==(JsonObject rhs) const {
    return JsonObjectConst(data_) == JsonObjectConst(rhs.data_);
  }
  template <typename TString>
  FORCE_INLINE
      typename detail::enable_if<detail::IsString<TString>::value,
                                 detail::MemberProxy<JsonObject, TString>>::type
      operator[](const TString& key) const {
    return {*this, key};
  }
  template <typename TChar>
  FORCE_INLINE
      typename detail::enable_if<detail::IsString<TChar*>::value,
                                 detail::MemberProxy<JsonObject, TChar*>>::type
      operator[](TChar* key) const {
    return {*this, key};
  }
  FORCE_INLINE void remove(iterator it) const {
    if (!data_)
      return;
    data_->removeSlot(it.slot_);
  }
  template <typename TString>
  FORCE_INLINE void remove(const TString& key) const {
    removeMember(detail::adaptString(key));
  }
  template <typename TChar>
  FORCE_INLINE void remove(TChar* key) const {
    removeMember(detail::adaptString(key));
  }
  template <typename TString>
  FORCE_INLINE
      typename detail::enable_if<detail::IsString<TString>::value, bool>::type
      containsKey(const TString& key) const {
    return getMember(detail::adaptString(key)) != 0;
  }
  template <typename TChar>
  FORCE_INLINE
      typename detail::enable_if<detail::IsString<TChar*>::value, bool>::type
      containsKey(TChar* key) const {
    return getMember(detail::adaptString(key)) != 0;
  }
  template <typename TString>
  FORCE_INLINE JsonArray createNestedArray(const TString& key) const;
  template <typename TChar>
  FORCE_INLINE JsonArray createNestedArray(TChar* key) const;
  template <typename TString>
  JsonObject createNestedObject(const TString& key) const {
    return operator[](key).template to<JsonObject>();
  }
  template <typename TChar>
  JsonObject createNestedObject(TChar* key) const {
    return operator[](key).template to<JsonObject>();
  }
 private:
  detail::MemoryPool* getPool() const {
    return pool_;
  }
  detail::VariantData* getData() const {
    return detail::collectionToVariant(data_);
  }
  detail::VariantData* getOrCreateData() const {
    return detail::collectionToVariant(data_);
  }
  template <typename TAdaptedString>
  inline detail::VariantData* getMember(TAdaptedString key) const {
    if (!data_)
      return 0;
    return data_->getMember(key);
  }
  template <typename TAdaptedString>
  void removeMember(TAdaptedString key) const {
    if (!data_)
      return;
    data_->removeMember(key);
  }
  detail::CollectionData* data_;
  detail::MemoryPool* pool_;
};
template <>
struct Converter<JsonObject> : private detail::VariantAttorney {
  static void toJson(JsonVariantConst src, JsonVariant dst) {
    variantCopyFrom(getData(dst), getData(src), getPool(dst));
  }
  static JsonObject fromJson(JsonVariant src) {
    auto data = getData(src);
    auto pool = getPool(src);
    return JsonObject(pool, data != 0 ? data->asObject() : 0);
  }
  static detail::InvalidConversion<JsonVariantConst, JsonObject> fromJson(
      JsonVariantConst);
  static bool checkJson(JsonVariantConst) {
    return false;
  }
  static bool checkJson(JsonVariant src) {
    auto data = getData(src);
    return data && data->isObject();
  }
};
class JsonDocument : public detail::VariantOperators<const JsonDocument&> {
  friend class detail::VariantAttorney;
 public:
  JsonDocument(const JsonDocument&) = delete;
  JsonDocument& operator=(const JsonDocument&) = delete;
  template <typename T>
  T as() {
    return getVariant().template as<T>();
  }
  template <typename T>
  T as() const {
    return getVariant().template as<T>();
  }
  void clear() {
    pool_.clear();
    data_.setNull();
  }
  template <typename T>
  bool is() {
    return getVariant().template is<T>();
  }
  template <typename T>
  bool is() const {
    return getVariant().template is<T>();
  }
  bool isNull() const {
    return getVariant().isNull();
  }
  size_t memoryUsage() const {
    return pool_.size();
  }
  bool overflowed() const {
    return pool_.overflowed();
  }
  size_t nesting() const {
    return variantNesting(&data_);
  }
  size_t capacity() const {
    return pool_.capacity();
  }
  size_t size() const {
    return data_.size();
  }
  bool set(const JsonDocument& src) {
    return to<JsonVariant>().set(src.as<JsonVariantConst>());
  }
  template <typename T>
  typename detail::enable_if<!detail::is_base_of<JsonDocument, T>::value,
                             bool>::type
  set(const T& src) {
    return to<JsonVariant>().set(src);
  }
  template <typename T>
  typename detail::VariantTo<T>::type to() {
    clear();
    return getVariant().template to<T>();
  }
  JsonArray createNestedArray() {
    return add().to<JsonArray>();
  }
  template <typename TChar>
  JsonArray createNestedArray(TChar* key) {
    return operator[](key).template to<JsonArray>();
  }
  template <typename TString>
  JsonArray createNestedArray(const TString& key) {
    return operator[](key).template to<JsonArray>();
  }
  JsonObject createNestedObject() {
    return add().to<JsonObject>();
  }
  template <typename TChar>
  JsonObject createNestedObject(TChar* key) {
    return operator[](key).template to<JsonObject>();
  }
  template <typename TString>
  JsonObject createNestedObject(const TString& key) {
    return operator[](key).template to<JsonObject>();
  }
  template <typename TChar>
  bool containsKey(TChar* key) const {
    return data_.getMember(detail::adaptString(key)) != 0;
  }
  template <typename TString>
  bool containsKey(const TString& key) const {
    return data_.getMember(detail::adaptString(key)) != 0;
  }
  template <typename TString>
  FORCE_INLINE typename detail::enable_if<
      detail::IsString<TString>::value,
      detail::MemberProxy<JsonDocument&, TString>>::type
  operator[](const TString& key) {
    return {*this, key};
  }
  template <typename TChar>
  FORCE_INLINE typename detail::enable_if<
      detail::IsString<TChar*>::value,
      detail::MemberProxy<JsonDocument&, TChar*>>::type
  operator[](TChar* key) {
    return {*this, key};
  }
  template <typename TString>
  FORCE_INLINE typename detail::enable_if<detail::IsString<TString>::value,
                                          JsonVariantConst>::type
  operator[](const TString& key) const {
    return JsonVariantConst(data_.getMember(detail::adaptString(key)));
  }
  template <typename TChar>
  FORCE_INLINE typename detail::enable_if<detail::IsString<TChar*>::value,
                                          JsonVariantConst>::type
  operator[](TChar* key) const {
    return JsonVariantConst(data_.getMember(detail::adaptString(key)));
  }
  FORCE_INLINE detail::ElementProxy<JsonDocument&> operator[](size_t index) {
    return {*this, index};
  }
  FORCE_INLINE JsonVariantConst operator[](size_t index) const {
    return JsonVariantConst(data_.getElement(index));
  }
  FORCE_INLINE JsonVariant add() {
    return JsonVariant(&pool_, data_.addElement(&pool_));
  }
  template <typename TValue>
  FORCE_INLINE bool add(const TValue& value) {
    return add().set(value);
  }
  template <typename TChar>
  FORCE_INLINE bool add(TChar* value) {
    return add().set(value);
  }
  FORCE_INLINE void remove(size_t index) {
    data_.remove(index);
  }
  template <typename TChar>
  FORCE_INLINE typename detail::enable_if<detail::IsString<TChar*>::value>::type
  remove(TChar* key) {
    data_.remove(detail::adaptString(key));
  }
  template <typename TString>
  FORCE_INLINE
      typename detail::enable_if<detail::IsString<TString>::value>::type
      remove(const TString& key) {
    data_.remove(detail::adaptString(key));
  }
  FORCE_INLINE operator JsonVariant() {
    return getVariant();
  }
  FORCE_INLINE operator JsonVariantConst() const {
    return getVariant();
  }
 protected:
  JsonDocument() : pool_(0, 0) {}
  JsonDocument(detail::MemoryPool pool) : pool_(pool) {}
  JsonDocument(char* buf, size_t capa) : pool_(buf, capa) {}
  ~JsonDocument() {}
  void replacePool(detail::MemoryPool pool) {
    pool_ = pool;
  }
  JsonVariant getVariant() {
    return JsonVariant(&pool_, &data_);
  }
  JsonVariantConst getVariant() const {
    return JsonVariantConst(&data_);
  }
  detail::MemoryPool pool_;
  detail::VariantData data_;
 protected:
  detail::MemoryPool* getPool() {
    return &pool_;
  }
  detail::VariantData* getData() {
    return &data_;
  }
  const detail::VariantData* getData() const {
    return &data_;
  }
  detail::VariantData* getOrCreateData() {
    return &data_;
  }
};
inline void convertToJson(const JsonDocument& src, JsonVariant dst) {
  dst.set(src.as<JsonVariantConst>());
}
template <typename TAllocator>
class AllocatorOwner {
 public:
  AllocatorOwner() {}
  AllocatorOwner(TAllocator a) : allocator_(a) {}
  void* allocate(size_t size) {
    return allocator_.allocate(size);
  }
  void deallocate(void* ptr) {
    if (ptr)
      allocator_.deallocate(ptr);
  }
  void* reallocate(void* ptr, size_t new_size) {
    return allocator_.reallocate(ptr, new_size);
  }
  TAllocator& allocator() {
    return allocator_;
  }
 private:
  TAllocator allocator_;
};
template <typename TAllocator>
class BasicJsonDocument : AllocatorOwner<TAllocator>, public JsonDocument {
 public:
  explicit BasicJsonDocument(size_t capa, TAllocator alloc = TAllocator())
      : AllocatorOwner<TAllocator>(alloc), JsonDocument(allocPool(capa)) {}
  BasicJsonDocument(const BasicJsonDocument& src)
      : AllocatorOwner<TAllocator>(src), JsonDocument() {
    copyAssignFrom(src);
  }
  BasicJsonDocument(BasicJsonDocument&& src) : AllocatorOwner<TAllocator>(src) {
    moveAssignFrom(src);
  }
  BasicJsonDocument(const JsonDocument& src) {
    copyAssignFrom(src);
  }
  template <typename T>
  BasicJsonDocument(const T& src,
                    typename detail::enable_if<
                        detail::is_same<T, JsonVariant>::value ||
                        detail::is_same<T, JsonVariantConst>::value ||
                        detail::is_same<T, JsonArray>::value ||
                        detail::is_same<T, JsonArrayConst>::value ||
                        detail::is_same<T, JsonObject>::value ||
                        detail::is_same<T, JsonObjectConst>::value>::type* = 0)
      : JsonDocument(allocPool(src.memoryUsage())) {
    set(src);
  }
  BasicJsonDocument(JsonVariant src)
      : JsonDocument(allocPool(src.memoryUsage())) {
    set(src);
  }
  ~BasicJsonDocument() {
    freePool();
  }
  BasicJsonDocument& operator=(const BasicJsonDocument& src) {
    copyAssignFrom(src);
    return *this;
  }
  BasicJsonDocument& operator=(BasicJsonDocument&& src) {
    moveAssignFrom(src);
    return *this;
  }
  template <typename T>
  BasicJsonDocument& operator=(const T& src) {
    size_t requiredSize = src.memoryUsage();
    if (requiredSize > capacity())
      reallocPool(requiredSize);
    set(src);
    return *this;
  }
  void shrinkToFit() {
    ptrdiff_t bytes_reclaimed = pool_.squash();
    if (bytes_reclaimed == 0)
      return;
    void* old_ptr = pool_.buffer();
    void* new_ptr = this->reallocate(old_ptr, pool_.capacity());
    ptrdiff_t ptr_offset =
        static_cast<char*>(new_ptr) - static_cast<char*>(old_ptr);
    pool_.movePointers(ptr_offset);
    data_.movePointers(ptr_offset, ptr_offset - bytes_reclaimed);
  }
  bool garbageCollect() {
    BasicJsonDocument tmp(*this);
    if (!tmp.capacity())
      return false;
    moveAssignFrom(tmp);
    return true;
  }
  using AllocatorOwner<TAllocator>::allocator;
 private:
  detail::MemoryPool allocPool(size_t requiredSize) {
    size_t capa = detail::addPadding(requiredSize);
    return {reinterpret_cast<char*>(this->allocate(capa)), capa};
  }
  void reallocPool(size_t requiredSize) {
    size_t capa = detail::addPadding(requiredSize);
    if (capa == pool_.capacity())
      return;
    freePool();
    replacePool(allocPool(detail::addPadding(requiredSize)));
  }
  void freePool() {
    this->deallocate(getPool()->buffer());
  }
  void copyAssignFrom(const JsonDocument& src) {
    reallocPool(src.capacity());
    set(src);
  }
  void moveAssignFrom(BasicJsonDocument& src) {
    freePool();
    data_ = src.data_;
    pool_ = src.pool_;
    src.data_.setNull();
    src.pool_ = {0, 0};
  }
};
struct DefaultAllocator {
  void* allocate(size_t size) {
    return malloc(size);
  }
  void deallocate(void* ptr) {
    free(ptr);
  }
  void* reallocate(void* ptr, size_t new_size) {
    return realloc(ptr, new_size);
  }
};
typedef BasicJsonDocument<DefaultAllocator> DynamicJsonDocument;
template <size_t desiredCapacity>
class StaticJsonDocument : public JsonDocument {
  static const size_t capacity_ =
      detail::AddPadding<detail::Max<1, desiredCapacity>::value>::value;
 public:
  StaticJsonDocument() : JsonDocument(buffer_, capacity_) {}
  StaticJsonDocument(const StaticJsonDocument& src)
      : JsonDocument(buffer_, capacity_) {
    set(src);
  }
  template <typename T>
  StaticJsonDocument(
      const T& src,
      typename detail::enable_if<
          detail::is_convertible<T, JsonVariantConst>::value>::type* = 0)
      : JsonDocument(buffer_, capacity_) {
    set(src);
  }
  StaticJsonDocument(JsonVariant src) : JsonDocument(buffer_, capacity_) {
    set(src);
  }
  StaticJsonDocument& operator=(const StaticJsonDocument& src) {
    set(src);
    return *this;
  }
  template <typename T>
  StaticJsonDocument& operator=(const T& src) {
    set(src);
    return *this;
  }
  void garbageCollect() {
    StaticJsonDocument tmp(*this);
    set(tmp);
  }
 private:
  char buffer_[capacity_];
};
inline JsonObject JsonArray::createNestedObject() const {
  return add().to<JsonObject>();
}
ARDUINOJSON_END_PUBLIC_NAMESPACE
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <typename TDerived>
inline JsonArray VariantRefBase<TDerived>::createNestedArray() const {
  return add().template to<JsonArray>();
}
template <typename TDerived>
inline JsonObject VariantRefBase<TDerived>::createNestedObject() const {
  return add().template to<JsonObject>();
}
template <typename TDerived>
inline ElementProxy<TDerived> VariantRefBase<TDerived>::operator[](
    size_t index) const {
  return ElementProxy<TDerived>(derived(), index);
}
ARDUINOJSON_END_PRIVATE_NAMESPACE
ARDUINOJSON_BEGIN_PUBLIC_NAMESPACE
template <typename T>
inline typename detail::enable_if<!detail::is_array<T>::value, bool>::type
copyArray(const T& src, JsonVariant dst) {
  return dst.set(src);
}
template <typename T, size_t N, typename TDestination>
inline typename detail::enable_if<
    !detail::is_base_of<JsonDocument, TDestination>::value, bool>::type
copyArray(T (&src)[N], const TDestination& dst) {
  return copyArray(src, N, dst);
}
template <typename T, typename TDestination>
inline typename detail::enable_if<
    !detail::is_base_of<JsonDocument, TDestination>::value, bool>::type
copyArray(const T* src, size_t len, const TDestination& dst) {
  bool ok = true;
  for (size_t i = 0; i < len; i++) {
    ok &= copyArray(src[i], dst.add());
  }
  return ok;
}
template <typename TDestination>
inline bool copyArray(const char* src, size_t, const TDestination& dst) {
  return dst.set(src);
}
template <typename T>
inline bool copyArray(const T& src, JsonDocument& dst) {
  return copyArray(src, dst.to<JsonArray>());
}
template <typename T>
inline bool copyArray(const T* src, size_t len, JsonDocument& dst) {
  return copyArray(src, len, dst.to<JsonArray>());
}
template <typename T>
inline typename detail::enable_if<!detail::is_array<T>::value, size_t>::type
copyArray(JsonVariantConst src, T& dst) {
  dst = src.as<T>();
  return 1;
}
template <typename T, size_t N>
inline size_t copyArray(JsonArrayConst src, T (&dst)[N]) {
  return copyArray(src, dst, N);
}
template <typename T>
inline size_t copyArray(JsonArrayConst src, T* dst, size_t len) {
  size_t i = 0;
  for (JsonArrayConst::iterator it = src.begin(); it != src.end() && i < len;
       ++it)
    copyArray(*it, dst[i++]);
  return i;
}
template <size_t N>
inline size_t copyArray(JsonVariantConst src, char (&dst)[N]) {
  JsonString s = src;
  size_t len = N - 1;
  if (len > s.size())
    len = s.size();
  memcpy(dst, s.c_str(), len);
  dst[len] = 0;
  return 1;
}
template <typename TSource, typename T>
inline typename detail::enable_if<
    detail::is_array<T>::value &&
        detail::is_base_of<JsonDocument, TSource>::value,
    size_t>::type
copyArray(const TSource& src, T& dst) {
  return copyArray(src.template as<JsonArrayConst>(), dst);
}
ARDUINOJSON_END_PUBLIC_NAMESPACE
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
inline VariantSlot* CollectionData::addSlot(MemoryPool* pool) {
  VariantSlot* slot = pool->allocVariant();
  if (!slot)
    return 0;
  if (tail_) {
    ARDUINOJSON_ASSERT(pool->owns(tail_));  // Can't alter a linked array/object
    tail_->setNextNotNull(slot);
    tail_ = slot;
  } else {
    head_ = slot;
    tail_ = slot;
  }
  slot->clear();
  return slot;
}
inline VariantData* CollectionData::addElement(MemoryPool* pool) {
  return slotData(addSlot(pool));
}
template <typename TAdaptedString>
inline VariantData* CollectionData::addMember(TAdaptedString key,
                                              MemoryPool* pool) {
  VariantSlot* slot = addSlot(pool);
  if (!slotSetKey(slot, key, pool)) {
    removeSlot(slot);
    return 0;
  }
  return slot->data();
}
inline void CollectionData::clear() {
  head_ = 0;
  tail_ = 0;
}
template <typename TAdaptedString>
inline bool CollectionData::containsKey(const TAdaptedString& key) const {
  return getSlot(key) != 0;
}
inline bool CollectionData::copyFrom(const CollectionData& src,
                                     MemoryPool* pool) {
  clear();
  for (VariantSlot* s = src.head_; s; s = s->next()) {
    VariantData* var;
    if (s->key() != 0) {
      JsonString key(s->key(),
                     s->ownsKey() ? JsonString::Copied : JsonString::Linked);
      var = addMember(adaptString(key), pool);
    } else {
      var = addElement(pool);
    }
    if (!var)
      return false;
    if (!var->copyFrom(*s->data(), pool))
      return false;
  }
  return true;
}
template <typename TAdaptedString>
inline VariantSlot* CollectionData::getSlot(TAdaptedString key) const {
  if (key.isNull())
    return 0;
  VariantSlot* slot = head_;
  while (slot) {
    if (stringEquals(key, adaptString(slot->key())))
      break;
    slot = slot->next();
  }
  return slot;
}
inline VariantSlot* CollectionData::getSlot(size_t index) const {
  if (!head_)
    return 0;
  return head_->next(index);
}
inline VariantSlot* CollectionData::getPreviousSlot(VariantSlot* target) const {
  VariantSlot* current = head_;
  while (current) {
    VariantSlot* next = current->next();
    if (next == target)
      return current;
    current = next;
  }
  return 0;
}
template <typename TAdaptedString>
inline VariantData* CollectionData::getMember(TAdaptedString key) const {
  VariantSlot* slot = getSlot(key);
  return slot ? slot->data() : 0;
}
template <typename TAdaptedString>
inline VariantData* CollectionData::getOrAddMember(TAdaptedString key,
                                                   MemoryPool* pool) {
  if (key.isNull())
    return 0;
  VariantSlot* slot = getSlot(key);
  if (slot)
    return slot->data();
  return addMember(key, pool);
}
inline VariantData* CollectionData::getElement(size_t index) const {
  VariantSlot* slot = getSlot(index);
  return slot ? slot->data() : 0;
}
inline VariantData* CollectionData::getOrAddElement(size_t index,
                                                    MemoryPool* pool) {
  VariantSlot* slot = head_;
  while (slot && index > 0) {
    slot = slot->next();
    index--;
  }
  if (!slot)
    index++;
  while (index > 0) {
    slot = addSlot(pool);
    index--;
  }
  return slotData(slot);
}
inline void CollectionData::removeSlot(VariantSlot* slot) {
  if (!slot)
    return;
  VariantSlot* prev = getPreviousSlot(slot);
  VariantSlot* next = slot->next();
  if (prev)
    prev->setNext(next);
  else
    head_ = next;
  if (!next)
    tail_ = prev;
}
inline void CollectionData::removeElement(size_t index) {
  removeSlot(getSlot(index));
}
inline size_t CollectionData::memoryUsage() const {
  size_t total = 0;
  for (VariantSlot* s = head_; s; s = s->next()) {
    total += sizeof(VariantSlot) + s->data()->memoryUsage();
    if (s->ownsKey())
      total += strlen(s->key()) + 1;
  }
  return total;
}
inline size_t CollectionData::size() const {
  return slotSize(head_);
}
template <typename T>
inline void movePointer(T*& p, ptrdiff_t offset) {
  if (!p)
    return;
  p = reinterpret_cast<T*>(
      reinterpret_cast<void*>(reinterpret_cast<char*>(p) + offset));
  ARDUINOJSON_ASSERT(isAligned(p));
}
inline void CollectionData::movePointers(ptrdiff_t stringDistance,
                                         ptrdiff_t variantDistance) {
  movePointer(head_, variantDistance);
  movePointer(tail_, variantDistance);
  for (VariantSlot* slot = head_; slot; slot = slot->next())
    slot->movePointers(stringDistance, variantDistance);
}
ARDUINOJSON_END_PRIVATE_NAMESPACE
ARDUINOJSON_BEGIN_PUBLIC_NAMESPACE
template <typename TString>
inline JsonArray JsonObject::createNestedArray(const TString& key) const {
  return operator[](key).template to<JsonArray>();
}
template <typename TChar>
inline JsonArray JsonObject::createNestedArray(TChar* key) const {
  return operator[](key).template to<JsonArray>();
}
ARDUINOJSON_END_PUBLIC_NAMESPACE
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <typename TDerived>
template <typename TString>
inline JsonArray VariantRefBase<TDerived>::createNestedArray(
    const TString& key) const {
  return operator[](key).template to<JsonArray>();
}
template <typename TDerived>
template <typename TChar>
inline JsonArray VariantRefBase<TDerived>::createNestedArray(TChar* key) const {
  return operator[](key).template to<JsonArray>();
}
template <typename TDerived>
template <typename TString>
inline JsonObject VariantRefBase<TDerived>::createNestedObject(
    const TString& key) const {
  return operator[](key).template to<JsonObject>();
}
template <typename TDerived>
template <typename TChar>
inline JsonObject VariantRefBase<TDerived>::createNestedObject(
    TChar* key) const {
  return operator[](key).template to<JsonObject>();
}
template <typename TDerived>
template <typename TString>
inline typename enable_if<IsString<TString>::value, bool>::type
VariantRefBase<TDerived>::containsKey(const TString& key) const {
  return variantGetMember(VariantAttorney::getData(derived()),
                          adaptString(key)) != 0;
}
template <typename TDerived>
template <typename TChar>
inline typename enable_if<IsString<TChar*>::value, bool>::type
VariantRefBase<TDerived>::containsKey(TChar* key) const {
  return variantGetMember(VariantAttorney::getData(derived()),
                          adaptString(key)) != 0;
}
template <typename TDerived>
template <typename TString>
inline typename enable_if<IsString<TString*>::value,
                          MemberProxy<TDerived, TString*>>::type
VariantRefBase<TDerived>::operator[](TString* key) const {
  return MemberProxy<TDerived, TString*>(derived(), key);
}
template <typename TDerived>
template <typename TString>
inline typename enable_if<IsString<TString>::value,
                          MemberProxy<TDerived, TString>>::type
VariantRefBase<TDerived>::operator[](const TString& key) const {
  return MemberProxy<TDerived, TString>(derived(), key);
}
class EscapeSequence {
 public:
  static char escapeChar(char c) {
    const char* p = escapeTable(true);
    while (p[0] && p[1] != c) {
      p += 2;
    }
    return p[0];
  }
  static char unescapeChar(char c) {
    const char* p = escapeTable(false);
    for (;;) {
      if (p[0] == '\0')
        return 0;
      if (p[0] == c)
        return p[1];
      p += 2;
    }
  }
 private:
  static const char* escapeTable(bool excludeSolidus) {
    return &"//\"\"\\\\b\bf\fn\nr\rt\t"[excludeSolidus ? 2 : 0];
  }
};
template <typename TFloat>
struct FloatParts {
  uint32_t integral;
  uint32_t decimal;
  int16_t exponent;
  int8_t decimalPlaces;
  FloatParts(TFloat value) {
    uint32_t maxDecimalPart = sizeof(TFloat) >= 8 ? 1000000000 : 1000000;
    decimalPlaces = sizeof(TFloat) >= 8 ? 9 : 6;
    exponent = normalize(value);
    integral = uint32_t(value);
    for (uint32_t tmp = integral; tmp >= 10; tmp /= 10) {
      maxDecimalPart /= 10;
      decimalPlaces--;
    }
    TFloat remainder = (value - TFloat(integral)) * TFloat(maxDecimalPart);
    decimal = uint32_t(remainder);
    remainder = remainder - TFloat(decimal);
    decimal += uint32_t(remainder * 2);
    if (decimal >= maxDecimalPart) {
      decimal = 0;
      integral++;
      if (exponent && integral >= 10) {
        exponent++;
        integral = 1;
      }
    }
    while (decimal % 10 == 0 && decimalPlaces > 0) {
      decimal /= 10;
      decimalPlaces--;
    }
  }
  static int16_t normalize(TFloat& value) {
    typedef FloatTraits<TFloat> traits;
    int16_t powersOf10 = 0;
    int8_t index = sizeof(TFloat) == 8 ? 8 : 5;
    int bit = 1 << index;
    if (value >= ARDUINOJSON_POSITIVE_EXPONENTIATION_THRESHOLD) {
      for (; index >= 0; index--) {
        if (value >= traits::positiveBinaryPowersOfTen()[index]) {
          value *= traits::negativeBinaryPowersOfTen()[index];
          powersOf10 = int16_t(powersOf10 + bit);
        }
        bit >>= 1;
      }
    }
    if (value > 0 && value <= ARDUINOJSON_NEGATIVE_EXPONENTIATION_THRESHOLD) {
      for (; index >= 0; index--) {
        if (value < traits::negativeBinaryPowersOfTen()[index] * 10) {
          value *= traits::positiveBinaryPowersOfTen()[index];
          powersOf10 = int16_t(powersOf10 - bit);
        }
        bit >>= 1;
      }
    }
    return powersOf10;
  }
};
template <typename TWriter>
class CountingDecorator {
 public:
  explicit CountingDecorator(TWriter& writer) : writer_(writer), count_(0) {}
  void write(uint8_t c) {
    count_ += writer_.write(c);
  }
  void write(const uint8_t* s, size_t n) {
    count_ += writer_.write(s, n);
  }
  size_t count() const {
    return count_;
  }
 private:
  TWriter writer_;
  size_t count_;
};
template <typename TWriter>
class TextFormatter {
 public:
  explicit TextFormatter(TWriter writer) : writer_(writer) {}
  TextFormatter& operator=(const TextFormatter&) = delete;
  size_t bytesWritten() const {
    return writer_.count();
  }
  void writeBoolean(bool value) {
    if (value)
      writeRaw("true");
    else
      writeRaw("false");
  }
  void writeString(const char* value) {
    ARDUINOJSON_ASSERT(value != NULL);
    writeRaw('\"');
    while (*value)
      writeChar(*value++);
    writeRaw('\"');
  }
  void writeString(const char* value, size_t n) {
    ARDUINOJSON_ASSERT(value != NULL);
    writeRaw('\"');
    while (n--)
      writeChar(*value++);
    writeRaw('\"');
  }
  void writeChar(char c) {
    char specialChar = EscapeSequence::escapeChar(c);
    if (specialChar) {
      writeRaw('\\');
      writeRaw(specialChar);
    } else if (c) {
      writeRaw(c);
    } else {
      writeRaw("\\u0000");
    }
  }
  template <typename T>
  void writeFloat(T value) {
    if (isnan(value))
      return writeRaw(ARDUINOJSON_ENABLE_NAN ? "NaN" : "null");
#if ARDUINOJSON_ENABLE_INFINITY
    if (value < 0.0) {
      writeRaw('-');
      value = -value;
    }
    if (isinf(value))
      return writeRaw("Infinity");
#else
    if (isinf(value))
      return writeRaw("null");
    if (value < 0.0) {
      writeRaw('-');
      value = -value;
    }
#endif
    FloatParts<T> parts(value);
    writeInteger(parts.integral);
    if (parts.decimalPlaces)
      writeDecimals(parts.decimal, parts.decimalPlaces);
    if (parts.exponent) {
      writeRaw('e');
      writeInteger(parts.exponent);
    }
  }
  template <typename T>
  typename enable_if<is_signed<T>::value>::type writeInteger(T value) {
    typedef typename make_unsigned<T>::type unsigned_type;
    unsigned_type unsigned_value;
    if (value < 0) {
      writeRaw('-');
      unsigned_value = unsigned_type(unsigned_type(~value) + 1);
    } else {
      unsigned_value = unsigned_type(value);
    }
    writeInteger(unsigned_value);
  }
  template <typename T>
  typename enable_if<is_unsigned<T>::value>::type writeInteger(T value) {
    char buffer[22];
    char* end = buffer + sizeof(buffer);
    char* begin = end;
    do {
      *--begin = char(value % 10 + '0');
      value = T(value / 10);
    } while (value);
    writeRaw(begin, end);
  }
  void writeDecimals(uint32_t value, int8_t width) {
    char buffer[16];
    char* end = buffer + sizeof(buffer);
    char* begin = end;
    while (width--) {
      *--begin = char(value % 10 + '0');
      value /= 10;
    }
    *--begin = '.';
    writeRaw(begin, end);
  }
  void writeRaw(const char* s) {
    writer_.write(reinterpret_cast<const uint8_t*>(s), strlen(s));
  }
  void writeRaw(const char* s, size_t n) {
    writer_.write(reinterpret_cast<const uint8_t*>(s), n);
  }
  void writeRaw(const char* begin, const char* end) {
    writer_.write(reinterpret_cast<const uint8_t*>(begin),
                  static_cast<size_t>(end - begin));
  }
  template <size_t N>
  void writeRaw(const char (&s)[N]) {
    writer_.write(reinterpret_cast<const uint8_t*>(s), N - 1);
  }
  void writeRaw(char c) {
    writer_.write(static_cast<uint8_t>(c));
  }
 protected:
  CountingDecorator<TWriter> writer_;
};
class DummyWriter {
 public:
  size_t write(uint8_t) {
    return 1;
  }
  size_t write(const uint8_t*, size_t n) {
    return n;
  }
};
template <template <typename> class TSerializer>
size_t measure(ArduinoJson::JsonVariantConst source) {
  DummyWriter dp;
  TSerializer<DummyWriter> serializer(dp);
  return variantAccept(VariantAttorney::getData(source), serializer);
}
template <typename TDestination, typename Enable = void>
class Writer {
 public:
  explicit Writer(TDestination& dest) : dest_(&dest) {}
  size_t write(uint8_t c) {
    return dest_->write(c);
  }
  size_t write(const uint8_t* s, size_t n) {
    return dest_->write(s, n);
  }
 private:
  TDestination* dest_;
};
class StaticStringWriter {
 public:
  StaticStringWriter(char* buf, size_t size) : end(buf + size), p(buf) {}
  size_t write(uint8_t c) {
    if (p >= end)
      return 0;
    *p++ = static_cast<char>(c);
    return 1;
  }
  size_t write(const uint8_t* s, size_t n) {
    char* begin = p;
    while (p < end && n > 0) {
      *p++ = static_cast<char>(*s++);
      n--;
    }
    return size_t(p - begin);
  }
 private:
  char* end;
  char* p;
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
#if ARDUINOJSON_ENABLE_STD_STRING
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <class...>
using void_t = void;
template <class T, typename = void>
struct is_std_string : false_type {};
template <class T>
struct is_std_string<
    T, void_t<decltype(T().push_back('a')), decltype(T().append(""))>>
    : true_type {};
template <typename TDestination>
class Writer<TDestination,
             typename enable_if<is_std_string<TDestination>::value>::type> {
 public:
  Writer(TDestination& str) : str_(&str) {}
  size_t write(uint8_t c) {
    str_->push_back(static_cast<char>(c));
    return 1;
  }
  size_t write(const uint8_t* s, size_t n) {
    str_->append(reinterpret_cast<const char*>(s), n);
    return n;
  }
 private:
  TDestination* str_;
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
#endif
#if ARDUINOJSON_ENABLE_ARDUINO_STRING
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <>
class Writer<::String, void> {
  static const size_t bufferCapacity = ARDUINOJSON_STRING_BUFFER_SIZE;
 public:
  explicit Writer(::String& str) : destination_(&str) {
    size_ = 0;
  }
  ~Writer() {
    flush();
  }
  size_t write(uint8_t c) {
    if (size_ + 1 >= bufferCapacity)
      if (flush() != 0)
        return 0;
    buffer_[size_++] = static_cast<char>(c);
    return 1;
  }
  size_t write(const uint8_t* s, size_t n) {
    for (size_t i = 0; i < n; i++) {
      write(s[i]);
    }
    return n;
  }
  size_t flush() {
    ARDUINOJSON_ASSERT(size_ < bufferCapacity);
    buffer_[size_] = 0;
    if (destination_->concat(buffer_))
      size_ = 0;
    return size_;
  }
 private:
  ::String* destination_;
  char buffer_[bufferCapacity];
  size_t size_;
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
#endif
#if ARDUINOJSON_ENABLE_STD_STREAM
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <typename TDestination>
class Writer<
    TDestination,
    typename enable_if<is_base_of<std::ostream, TDestination>::value>::type> {
 public:
  explicit Writer(std::ostream& os) : os_(&os) {}
  size_t write(uint8_t c) {
    os_->put(static_cast<char>(c));
    return 1;
  }
  size_t write(const uint8_t* s, size_t n) {
    os_->write(reinterpret_cast<const char*>(s),
               static_cast<std::streamsize>(n));
    return n;
  }
 private:
  std::ostream* os_;
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
#endif
#if ARDUINOJSON_ENABLE_ARDUINO_PRINT
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <typename TDestination>
class Writer<
    TDestination,
    typename enable_if<is_base_of<::Print, TDestination>::value>::type> {
 public:
  explicit Writer(::Print& print) : print_(&print) {}
  size_t write(uint8_t c) {
    return print_->write(c);
  }
  size_t write(const uint8_t* s, size_t n) {
    return print_->write(s, n);
  }
 private:
  ::Print* print_;
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
#endif
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <template <typename> class TSerializer, typename TWriter>
size_t doSerialize(ArduinoJson::JsonVariantConst source, TWriter writer) {
  TSerializer<TWriter> serializer(writer);
  return variantAccept(VariantAttorney::getData(source), serializer);
}
template <template <typename> class TSerializer, typename TDestination>
size_t serialize(ArduinoJson::JsonVariantConst source,
                 TDestination& destination) {
  Writer<TDestination> writer(destination);
  return doSerialize<TSerializer>(source, writer);
}
template <template <typename> class TSerializer>
typename enable_if<!TSerializer<StaticStringWriter>::producesText, size_t>::type
serialize(ArduinoJson::JsonVariantConst source, void* buffer,
          size_t bufferSize) {
  StaticStringWriter writer(reinterpret_cast<char*>(buffer), bufferSize);
  return doSerialize<TSerializer>(source, writer);
}
template <template <typename> class TSerializer>
typename enable_if<TSerializer<StaticStringWriter>::producesText, size_t>::type
serialize(ArduinoJson::JsonVariantConst source, void* buffer,
          size_t bufferSize) {
  StaticStringWriter writer(reinterpret_cast<char*>(buffer), bufferSize);
  size_t n = doSerialize<TSerializer>(source, writer);
  if (n < bufferSize)
    reinterpret_cast<char*>(buffer)[n] = 0;
  return n;
}
template <template <typename> class TSerializer, typename TChar, size_t N>
typename enable_if<IsChar<TChar>::value, size_t>::type serialize(
    ArduinoJson::JsonVariantConst source, TChar (&buffer)[N]) {
  return serialize<TSerializer>(source, buffer, N);
}
template <typename TWriter>
class JsonSerializer : public Visitor<size_t> {
 public:
  static const bool producesText = true;
  JsonSerializer(TWriter writer) : formatter_(writer) {}
  FORCE_INLINE size_t visitArray(const CollectionData& array) {
    write('[');
    const VariantSlot* slot = array.head();
    while (slot != 0) {
      slot->data()->accept(*this);
      slot = slot->next();
      if (slot == 0)
        break;
      write(',');
    }
    write(']');
    return bytesWritten();
  }
  size_t visitObject(const CollectionData& object) {
    write('{');
    const VariantSlot* slot = object.head();
    while (slot != 0) {
      formatter_.writeString(slot->key());
      write(':');
      slot->data()->accept(*this);
      slot = slot->next();
      if (slot == 0)
        break;
      write(',');
    }
    write('}');
    return bytesWritten();
  }
  size_t visitFloat(JsonFloat value) {
    formatter_.writeFloat(value);
    return bytesWritten();
  }
  size_t visitString(const char* value) {
    formatter_.writeString(value);
    return bytesWritten();
  }
  size_t visitString(const char* value, size_t n) {
    formatter_.writeString(value, n);
    return bytesWritten();
  }
  size_t visitRawJson(const char* data, size_t n) {
    formatter_.writeRaw(data, n);
    return bytesWritten();
  }
  size_t visitSignedInteger(JsonInteger value) {
    formatter_.writeInteger(value);
    return bytesWritten();
  }
  size_t visitUnsignedInteger(JsonUInt value) {
    formatter_.writeInteger(value);
    return bytesWritten();
  }
  size_t visitBoolean(bool value) {
    formatter_.writeBoolean(value);
    return bytesWritten();
  }
  size_t visitNull() {
    formatter_.writeRaw("null");
    return bytesWritten();
  }
 protected:
  size_t bytesWritten() const {
    return formatter_.bytesWritten();
  }
  void write(char c) {
    formatter_.writeRaw(c);
  }
  void write(const char* s) {
    formatter_.writeRaw(s);
  }
 private:
  TextFormatter<TWriter> formatter_;
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
ARDUINOJSON_BEGIN_PUBLIC_NAMESPACE
template <typename TDestination>
size_t serializeJson(JsonVariantConst source, TDestination& destination) {
  using namespace detail;
  return serialize<JsonSerializer>(source, destination);
}
inline size_t serializeJson(JsonVariantConst source, void* buffer,
                            size_t bufferSize) {
  using namespace detail;
  return serialize<JsonSerializer>(source, buffer, bufferSize);
}
inline size_t measureJson(JsonVariantConst source) {
  using namespace detail;
  return measure<JsonSerializer>(source);
}
#if ARDUINOJSON_ENABLE_STD_STREAM
template <typename T>
inline typename detail::enable_if<
    detail::is_convertible<T, JsonVariantConst>::value, std::ostream&>::type
operator<<(std::ostream& os, const T& source) {
  serializeJson(source, os);
  return os;
}
#endif
ARDUINOJSON_END_PUBLIC_NAMESPACE
#if ARDUINOJSON_ENABLE_STD_STRING
#include <string>
#endif
#if ARDUINOJSON_ENABLE_STRING_VIEW
#include <string_view>
#endif
ARDUINOJSON_BEGIN_PUBLIC_NAMESPACE
template <typename T, typename Enable>
struct Converter {
  static void toJson(const T& src, JsonVariant dst) {
    convertToJson(src, dst); // Error here? See https://arduinojson.org/v6/unsupported-set/
  }
  static T fromJson(JsonVariantConst src) {
    T result; // Error here? See https://arduinojson.org/v6/non-default-constructible/
    convertFromJson(src, result);  // Error here? See https://arduinojson.org/v6/unsupported-as/
    return result;
  }
  static bool checkJson(JsonVariantConst src) {
    T dummy = T();
    return canConvertFromJson(src, dummy);  // Error here? See https://arduinojson.org/v6/unsupported-is/
  }
};
template <typename T>
struct Converter<
    T, typename detail::enable_if<detail::is_integral<T>::value &&
                                  !detail::is_same<bool, T>::value &&
                                  !detail::is_same<char, T>::value>::type>
    : private detail::VariantAttorney {
  static void toJson(T src, JsonVariant dst) {
    auto data = getData(dst);
    ARDUINOJSON_ASSERT_INTEGER_TYPE_IS_SUPPORTED(T);
    if (data)
      data->setInteger(src);
  }
  static T fromJson(JsonVariantConst src) {
    ARDUINOJSON_ASSERT_INTEGER_TYPE_IS_SUPPORTED(T);
    auto data = getData(src);
    return data ? data->template asIntegral<T>() : T();
  }
  static bool checkJson(JsonVariantConst src) {
    auto data = getData(src);
    return data && data->template isInteger<T>();
  }
};
template <typename T>
struct Converter<T, typename detail::enable_if<detail::is_enum<T>::value>::type>
    : private detail::VariantAttorney {
  static void toJson(T src, JsonVariant dst) {
    dst.set(static_cast<JsonInteger>(src));
  }
  static T fromJson(JsonVariantConst src) {
    auto data = getData(src);
    return data ? static_cast<T>(data->template asIntegral<int>()) : T();
  }
  static bool checkJson(JsonVariantConst src) {
    auto data = getData(src);
    return data && data->template isInteger<int>();
  }
};
template <>
struct Converter<bool> : private detail::VariantAttorney {
  static void toJson(bool src, JsonVariant dst) {
    auto data = getData(dst);
    if (data)
      data->setBoolean(src);
  }
  static bool fromJson(JsonVariantConst src) {
    auto data = getData(src);
    return data ? data->asBoolean() : false;
  }
  static bool checkJson(JsonVariantConst src) {
    auto data = getData(src);
    return data && data->isBoolean();
  }
};
template <typename T>
struct Converter<
    T, typename detail::enable_if<detail::is_floating_point<T>::value>::type>
    : private detail::VariantAttorney {
  static void toJson(T src, JsonVariant dst) {
    auto data = getData(dst);
    if (data)
      data->setFloat(static_cast<JsonFloat>(src));
  }
  static T fromJson(JsonVariantConst src) {
    auto data = getData(src);
    return data ? data->template asFloat<T>() : 0;
  }
  static bool checkJson(JsonVariantConst src) {
    auto data = getData(src);
    return data && data->isFloat();
  }
};
template <>
struct Converter<const char*> : private detail::VariantAttorney {
  static void toJson(const char* src, JsonVariant dst) {
    variantSetString(getData(dst), detail::adaptString(src), getPool(dst));
  }
  static const char* fromJson(JsonVariantConst src) {
    auto data = getData(src);
    return data ? data->asString().c_str() : 0;
  }
  static bool checkJson(JsonVariantConst src) {
    auto data = getData(src);
    return data && data->isString();
  }
};
template <>
struct Converter<JsonString> : private detail::VariantAttorney {
  static void toJson(JsonString src, JsonVariant dst) {
    variantSetString(getData(dst), detail::adaptString(src), getPool(dst));
  }
  static JsonString fromJson(JsonVariantConst src) {
    auto data = getData(src);
    return data ? data->asString() : 0;
  }
  static bool checkJson(JsonVariantConst src) {
    auto data = getData(src);
    return data && data->isString();
  }
};
template <typename T>
inline typename detail::enable_if<detail::IsString<T>::value, bool>::type
convertToJson(const T& src, JsonVariant dst) {
  using namespace detail;
  auto data = VariantAttorney::getData(dst);
  auto pool = VariantAttorney::getPool(dst);
  return variantSetString(data, adaptString(src), pool);
}
template <>
struct Converter<SerializedValue<const char*>>
    : private detail::VariantAttorney {
  static void toJson(SerializedValue<const char*> src, JsonVariant dst) {
    auto data = getData(dst);
    if (data)
      data->setLinkedRaw(src);
  }
};
template <typename T>
struct Converter<
    SerializedValue<T>,
    typename detail::enable_if<!detail::is_same<const char*, T>::value>::type>
    : private detail::VariantAttorney {
  static void toJson(SerializedValue<T> src, JsonVariant dst) {
    auto data = getData(dst);
    auto pool = getPool(dst);
    if (data)
      data->storeOwnedRaw(src, pool);
  }
};
template <>
struct Converter<decltype(nullptr)> : private detail::VariantAttorney {
  static void toJson(decltype(nullptr), JsonVariant dst) {
    variantSetNull(getData(dst));
  }
  static decltype(nullptr) fromJson(JsonVariantConst) {
    return nullptr;
  }
  static bool checkJson(JsonVariantConst src) {
    auto data = getData(src);
    return data == 0 || data->isNull();
  }
};
#if ARDUINOJSON_ENABLE_ARDUINO_STREAM
namespace detail {
class MemoryPoolPrint : public Print {
 public:
  MemoryPoolPrint(MemoryPool* pool) : pool_(pool), size_(0) {
    pool->getFreeZone(&string_, &capacity_);
  }
  JsonString str() {
    ARDUINOJSON_ASSERT(size_ < capacity_);
    return JsonString(pool_->saveStringFromFreeZone(size_), size_,
                      JsonString::Copied);
  }
  size_t write(uint8_t c) {
    if (size_ >= capacity_)
      return 0;
    string_[size_++] = char(c);
    return 1;
  }
  size_t write(const uint8_t* buffer, size_t size) {
    if (size_ + size >= capacity_) {
      size_ = capacity_;  // mark as overflowed
      return 0;
    }
    memcpy(&string_[size_], buffer, size);
    size_ += size;
    return size;
  }
  bool overflowed() const {
    return size_ >= capacity_;
  }
 private:
  MemoryPool* pool_;
  size_t size_;
  char* string_;
  size_t capacity_;
};
}  // namespace detail
inline void convertToJson(const ::Printable& src, JsonVariant dst) {
  auto pool = detail::VariantAttorney::getPool(dst);
  auto data = detail::VariantAttorney::getData(dst);
  if (!pool || !data)
    return;
  detail::MemoryPoolPrint print(pool);
  src.printTo(print);
  if (print.overflowed()) {
    pool->markAsOverflowed();
    data->setNull();
    return;
  }
  data->setString(print.str());
}
#endif
#if ARDUINOJSON_ENABLE_ARDUINO_STRING
inline void convertFromJson(JsonVariantConst src, ::String& dst) {
  JsonString str = src.as<JsonString>();
  if (str)
    dst = str.c_str();
  else
    serializeJson(src, dst);
}
inline bool canConvertFromJson(JsonVariantConst src, const ::String&) {
  return src.is<JsonString>();
}
#endif
#if ARDUINOJSON_ENABLE_STD_STRING
inline void convertFromJson(JsonVariantConst src, std::string& dst) {
  JsonString str = src.as<JsonString>();
  if (str)
    dst.assign(str.c_str(), str.size());
  else
    serializeJson(src, dst);
}
inline bool canConvertFromJson(JsonVariantConst src, const std::string&) {
  return src.is<JsonString>();
}
#endif
#if ARDUINOJSON_ENABLE_STRING_VIEW
inline void convertFromJson(JsonVariantConst src, std::string_view& dst) {
  JsonString str = src.as<JsonString>();
  if (str)  // the standard doesn't allow passing null to the constructor
    dst = std::string_view(str.c_str(), str.size());
}
inline bool canConvertFromJson(JsonVariantConst src, const std::string_view&) {
  return src.is<JsonString>();
}
#endif
namespace detail {
template <typename T>
struct ConverterNeedsWriteableRef {
 protected:  // <- to avoid GCC's "all member functions in class are private"
  static int probe(T (*f)(ArduinoJson::JsonVariant));
  static char probe(T (*f)(ArduinoJson::JsonVariantConst));
 public:
  static const bool value =
      sizeof(probe(Converter<T>::fromJson)) == sizeof(int);
};
}  // namespace detail
ARDUINOJSON_END_PUBLIC_NAMESPACE
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
class CollectionData;
struct ComparerBase : Visitor<CompareResult> {};
template <typename T, typename Enable = void>
struct Comparer;
template <typename T>
struct Comparer<T, typename enable_if<IsString<T>::value>::type>
    : ComparerBase {
  T rhs;  // TODO: store adapted string?
  explicit Comparer(T value) : rhs(value) {}
  CompareResult visitString(const char* lhs, size_t n) {
    int i = stringCompare(adaptString(rhs), adaptString(lhs, n));
    if (i < 0)
      return COMPARE_RESULT_GREATER;
    else if (i > 0)
      return COMPARE_RESULT_LESS;
    else
      return COMPARE_RESULT_EQUAL;
  }
  CompareResult visitNull() {
    if (adaptString(rhs).isNull())
      return COMPARE_RESULT_EQUAL;
    else
      return COMPARE_RESULT_DIFFER;
  }
};
template <typename T>
struct Comparer<T, typename enable_if<is_integral<T>::value ||
                                      is_floating_point<T>::value>::type>
    : ComparerBase {
  T rhs;
  explicit Comparer(T value) : rhs(value) {}
  CompareResult visitFloat(JsonFloat lhs) {
    return arithmeticCompare(lhs, rhs);
  }
  CompareResult visitSignedInteger(JsonInteger lhs) {
    return arithmeticCompare(lhs, rhs);
  }
  CompareResult visitUnsignedInteger(JsonUInt lhs) {
    return arithmeticCompare(lhs, rhs);
  }
  CompareResult visitBoolean(bool lhs) {
    return visitUnsignedInteger(static_cast<JsonUInt>(lhs));
  }
};
struct NullComparer : ComparerBase {
  CompareResult visitNull() {
    return COMPARE_RESULT_EQUAL;
  }
};
template <>
struct Comparer<decltype(nullptr), void> : NullComparer {
  explicit Comparer(decltype(nullptr)) : NullComparer() {}
};
struct ArrayComparer : ComparerBase {
  const CollectionData* rhs_;
  explicit ArrayComparer(const CollectionData& rhs) : rhs_(&rhs) {}
  CompareResult visitArray(const CollectionData& lhs) {
    if (JsonArrayConst(&lhs) == JsonArrayConst(rhs_))
      return COMPARE_RESULT_EQUAL;
    else
      return COMPARE_RESULT_DIFFER;
  }
};
struct ObjectComparer : ComparerBase {
  const CollectionData* rhs_;
  explicit ObjectComparer(const CollectionData& rhs) : rhs_(&rhs) {}
  CompareResult visitObject(const CollectionData& lhs) {
    if (JsonObjectConst(&lhs) == JsonObjectConst(rhs_))
      return COMPARE_RESULT_EQUAL;
    else
      return COMPARE_RESULT_DIFFER;
  }
};
struct RawComparer : ComparerBase {
  const char* rhsData_;
  size_t rhsSize_;
  explicit RawComparer(const char* rhsData, size_t rhsSize)
      : rhsData_(rhsData), rhsSize_(rhsSize) {}
  CompareResult visitRawJson(const char* lhsData, size_t lhsSize) {
    size_t size = rhsSize_ < lhsSize ? rhsSize_ : lhsSize;
    int n = memcmp(lhsData, rhsData_, size);
    if (n < 0)
      return COMPARE_RESULT_LESS;
    else if (n > 0)
      return COMPARE_RESULT_GREATER;
    else
      return COMPARE_RESULT_EQUAL;
  }
};
struct VariantComparer : ComparerBase {
  const VariantData* rhs;
  explicit VariantComparer(const VariantData* value) : rhs(value) {}
  CompareResult visitArray(const CollectionData& lhs) {
    ArrayComparer comparer(lhs);
    return accept(comparer);
  }
  CompareResult visitObject(const CollectionData& lhs) {
    ObjectComparer comparer(lhs);
    return accept(comparer);
  }
  CompareResult visitFloat(JsonFloat lhs) {
    Comparer<JsonFloat> comparer(lhs);
    return accept(comparer);
  }
  CompareResult visitString(const char* lhs, size_t) {
    Comparer<const char*> comparer(lhs);
    return accept(comparer);
  }
  CompareResult visitRawJson(const char* lhsData, size_t lhsSize) {
    RawComparer comparer(lhsData, lhsSize);
    return accept(comparer);
  }
  CompareResult visitSignedInteger(JsonInteger lhs) {
    Comparer<JsonInteger> comparer(lhs);
    return accept(comparer);
  }
  CompareResult visitUnsignedInteger(JsonUInt lhs) {
    Comparer<JsonUInt> comparer(lhs);
    return accept(comparer);
  }
  CompareResult visitBoolean(bool lhs) {
    Comparer<bool> comparer(lhs);
    return accept(comparer);
  }
  CompareResult visitNull() {
    NullComparer comparer;
    return accept(comparer);
  }
 private:
  template <typename TComparer>
  CompareResult accept(TComparer& comparer) {
    CompareResult reversedResult = variantAccept(rhs, comparer);
    switch (reversedResult) {
      case COMPARE_RESULT_GREATER:
        return COMPARE_RESULT_LESS;
      case COMPARE_RESULT_LESS:
        return COMPARE_RESULT_GREATER;
      default:
        return reversedResult;
    }
  }
};
template <typename T>
struct Comparer<T, typename enable_if<is_convertible<
                       T, ArduinoJson::JsonVariantConst>::value>::type>
    : VariantComparer {
  explicit Comparer(const T& value)
      : VariantComparer(VariantAttorney::getData(value)) {}
};
template <typename T>
CompareResult compare(ArduinoJson::JsonVariantConst lhs, const T& rhs) {
  Comparer<T> comparer(rhs);
  return variantAccept(VariantAttorney::getData(lhs), comparer);
}
#ifndef isdigit
inline bool isdigit(char c) {
  return '0' <= c && c <= '9';
}
#endif
inline bool issign(char c) {
  return '-' == c || c == '+';
}
template <typename A, typename B>
struct choose_largest : conditional<(sizeof(A) > sizeof(B)), A, B> {};
inline bool parseNumber(const char* s, VariantData& result) {
  typedef FloatTraits<JsonFloat> traits;
  typedef choose_largest<traits::mantissa_type, JsonUInt>::type mantissa_t;
  typedef traits::exponent_type exponent_t;
  ARDUINOJSON_ASSERT(s != 0);
  bool is_negative = false;
  switch (*s) {
    case '-':
      is_negative = true;
      s++;
      break;
    case '+':
      s++;
      break;
  }
#if ARDUINOJSON_ENABLE_NAN
  if (*s == 'n' || *s == 'N') {
    result.setFloat(traits::nan());
    return true;
  }
#endif
#if ARDUINOJSON_ENABLE_INFINITY
  if (*s == 'i' || *s == 'I') {
    result.setFloat(is_negative ? -traits::inf() : traits::inf());
    return true;
  }
#endif
  if (!isdigit(*s) && *s != '.')
    return false;
  mantissa_t mantissa = 0;
  exponent_t exponent_offset = 0;
  const mantissa_t maxUint = JsonUInt(-1);
  while (isdigit(*s)) {
    uint8_t digit = uint8_t(*s - '0');
    if (mantissa > maxUint / 10)
      break;
    mantissa *= 10;
    if (mantissa > maxUint - digit)
      break;
    mantissa += digit;
    s++;
  }
  if (*s == '\0') {
    if (is_negative) {
      const mantissa_t sintMantissaMax = mantissa_t(1)
                                         << (sizeof(JsonInteger) * 8 - 1);
      if (mantissa <= sintMantissaMax) {
        result.setInteger(JsonInteger(~mantissa + 1));
        return true;
      }
    } else {
      result.setInteger(JsonUInt(mantissa));
      return true;
    }
  }
  while (mantissa > traits::mantissa_max) {
    mantissa /= 10;
    exponent_offset++;
  }
  while (isdigit(*s)) {
    exponent_offset++;
    s++;
  }
  if (*s == '.') {
    s++;
    while (isdigit(*s)) {
      if (mantissa < traits::mantissa_max / 10) {
        mantissa = mantissa * 10 + uint8_t(*s - '0');
        exponent_offset--;
      }
      s++;
    }
  }
  int exponent = 0;
  if (*s == 'e' || *s == 'E') {
    s++;
    bool negative_exponent = false;
    if (*s == '-') {
      negative_exponent = true;
      s++;
    } else if (*s == '+') {
      s++;
    }
    while (isdigit(*s)) {
      exponent = exponent * 10 + (*s - '0');
      if (exponent + exponent_offset > traits::exponent_max) {
        if (negative_exponent)
          result.setFloat(is_negative ? -0.0f : 0.0f);
        else
          result.setFloat(is_negative ? -traits::inf() : traits::inf());
        return true;
      }
      s++;
    }
    if (negative_exponent)
      exponent = -exponent;
  }
  exponent += exponent_offset;
  if (*s != '\0')
    return false;
  JsonFloat final_result =
      make_float(static_cast<JsonFloat>(mantissa), exponent);
  result.setFloat(is_negative ? -final_result : final_result);
  return true;
}
template <typename T>
inline T parseNumber(const char* s) {
  VariantData value;
  parseNumber(s, value);
  return Converter<T>::fromJson(JsonVariantConst(&value));
}
template <typename T>
inline T VariantData::asIntegral() const {
  switch (type()) {
    case VALUE_IS_BOOLEAN:
      return content_.asBoolean;
    case VALUE_IS_UNSIGNED_INTEGER:
      return convertNumber<T>(content_.asUnsignedInteger);
    case VALUE_IS_SIGNED_INTEGER:
      return convertNumber<T>(content_.asSignedInteger);
    case VALUE_IS_LINKED_STRING:
    case VALUE_IS_OWNED_STRING:
      return parseNumber<T>(content_.asString.data);
    case VALUE_IS_FLOAT:
      return convertNumber<T>(content_.asFloat);
    default:
      return 0;
  }
}
inline bool VariantData::asBoolean() const {
  switch (type()) {
    case VALUE_IS_BOOLEAN:
      return content_.asBoolean;
    case VALUE_IS_SIGNED_INTEGER:
    case VALUE_IS_UNSIGNED_INTEGER:
      return content_.asUnsignedInteger != 0;
    case VALUE_IS_FLOAT:
      return content_.asFloat != 0;
    case VALUE_IS_NULL:
      return false;
    default:
      return true;
  }
}
template <typename T>
inline T VariantData::asFloat() const {
  switch (type()) {
    case VALUE_IS_BOOLEAN:
      return static_cast<T>(content_.asBoolean);
    case VALUE_IS_UNSIGNED_INTEGER:
      return static_cast<T>(content_.asUnsignedInteger);
    case VALUE_IS_SIGNED_INTEGER:
      return static_cast<T>(content_.asSignedInteger);
    case VALUE_IS_LINKED_STRING:
    case VALUE_IS_OWNED_STRING:
      return parseNumber<T>(content_.asString.data);
    case VALUE_IS_FLOAT:
      return static_cast<T>(content_.asFloat);
    default:
      return 0;
  }
}
inline JsonString VariantData::asString() const {
  switch (type()) {
    case VALUE_IS_LINKED_STRING:
      return JsonString(content_.asString.data, content_.asString.size,
                        JsonString::Linked);
    case VALUE_IS_OWNED_STRING:
      return JsonString(content_.asString.data, content_.asString.size,
                        JsonString::Copied);
    default:
      return JsonString();
  }
}
inline bool VariantData::copyFrom(const VariantData& src, MemoryPool* pool) {
  switch (src.type()) {
    case VALUE_IS_ARRAY:
      return toArray().copyFrom(src.content_.asCollection, pool);
    case VALUE_IS_OBJECT:
      return toObject().copyFrom(src.content_.asCollection, pool);
    case VALUE_IS_OWNED_STRING: {
      JsonString value = src.asString();
      return setString(adaptString(value), pool);
    }
    case VALUE_IS_OWNED_RAW:
      return storeOwnedRaw(
          serialized(src.content_.asString.data, src.content_.asString.size),
          pool);
    default:
      setType(src.type());
      content_ = src.content_;
      return true;
  }
}
template <typename TDerived>
inline JsonVariant VariantRefBase<TDerived>::add() const {
  return JsonVariant(getPool(),
                     variantAddElement(getOrCreateData(), getPool()));
}
template <typename TDerived>
template <typename T>
inline typename enable_if<ConverterNeedsWriteableRef<T>::value, T>::type
VariantRefBase<TDerived>::as() const {
  return Converter<T>::fromJson(getVariant());
}
template <typename TDerived>
inline JsonVariant VariantRefBase<TDerived>::getVariant() const {
  return JsonVariant(getPool(), getData());
}
template <typename TDerived>
inline JsonVariant VariantRefBase<TDerived>::getOrCreateVariant() const {
  return JsonVariant(getPool(), getOrCreateData());
}
template <typename TDerived>
template <typename T>
inline typename enable_if<ConverterNeedsWriteableRef<T>::value, bool>::type
VariantRefBase<TDerived>::is() const {
  return Converter<T>::checkJson(getVariant());
}
template <typename TDerived>
template <typename T>
inline bool VariantRefBase<TDerived>::set(const T& value) const {
  Converter<typename detail::remove_cv<T>::type>::toJson(value,
                                                         getOrCreateVariant());
  MemoryPool* pool = getPool();
  return pool && !pool->overflowed();
}
template <typename TDerived>
template <typename T>
inline bool VariantRefBase<TDerived>::set(T* value) const {
  Converter<T*>::toJson(value, getOrCreateVariant());
  MemoryPool* pool = getPool();
  return pool && !pool->overflowed();
}
template <typename TDerived>
template <typename T>
inline typename enable_if<is_same<T, JsonArray>::value, JsonArray>::type
VariantRefBase<TDerived>::to() const {
  return JsonArray(getPool(), variantToArray(getOrCreateData()));
}
template <typename TDerived>
template <typename T>
typename enable_if<is_same<T, JsonObject>::value, JsonObject>::type
VariantRefBase<TDerived>::to() const {
  return JsonObject(getPool(), variantToObject(getOrCreateData()));
}
template <typename TDerived>
template <typename T>
typename enable_if<is_same<T, JsonVariant>::value, JsonVariant>::type
VariantRefBase<TDerived>::to() const {
  auto data = getOrCreateData();
  variantSetNull(data);
  return JsonVariant(getPool(), data);
}
template <typename TDerived>
inline void convertToJson(const VariantRefBase<TDerived>& src,
                          JsonVariant dst) {
  dst.set(src.template as<JsonVariantConst>());
}
ARDUINOJSON_END_PRIVATE_NAMESPACE
#if ARDUINOJSON_ENABLE_STD_STREAM
#endif
ARDUINOJSON_BEGIN_PUBLIC_NAMESPACE
class DeserializationError {
 public:
  enum Code {
    Ok,
    EmptyInput,
    IncompleteInput,
    InvalidInput,
    NoMemory,
    TooDeep
  };
  DeserializationError() {}
  DeserializationError(Code c) : code_(c) {}
  friend bool operator==(const DeserializationError& lhs,
                         const DeserializationError& rhs) {
    return lhs.code_ == rhs.code_;
  }
  friend bool operator!=(const DeserializationError& lhs,
                         const DeserializationError& rhs) {
    return lhs.code_ != rhs.code_;
  }
  friend bool operator==(const DeserializationError& lhs, Code rhs) {
    return lhs.code_ == rhs;
  }
  friend bool operator==(Code lhs, const DeserializationError& rhs) {
    return lhs == rhs.code_;
  }
  friend bool operator!=(const DeserializationError& lhs, Code rhs) {
    return lhs.code_ != rhs;
  }
  friend bool operator!=(Code lhs, const DeserializationError& rhs) {
    return lhs != rhs.code_;
  }
  explicit operator bool() const {
    return code_ != Ok;
  }
  Code code() const {
    return code_;
  }
  const char* c_str() const {
    static const char* messages[] = {
        "Ok",           "EmptyInput", "IncompleteInput",
        "InvalidInput", "NoMemory",   "TooDeep"};
    ARDUINOJSON_ASSERT(static_cast<size_t>(code_) <
                       sizeof(messages) / sizeof(messages[0]));
    return messages[code_];
  }
#if ARDUINOJSON_ENABLE_PROGMEM
  const __FlashStringHelper* f_str() const {
    ARDUINOJSON_DEFINE_PROGMEM_ARRAY(char, s0, "Ok");
    ARDUINOJSON_DEFINE_PROGMEM_ARRAY(char, s1, "EmptyInput");
    ARDUINOJSON_DEFINE_PROGMEM_ARRAY(char, s2, "IncompleteInput");
    ARDUINOJSON_DEFINE_PROGMEM_ARRAY(char, s3, "InvalidInput");
    ARDUINOJSON_DEFINE_PROGMEM_ARRAY(char, s4, "NoMemory");
    ARDUINOJSON_DEFINE_PROGMEM_ARRAY(char, s5, "TooDeep");
    ARDUINOJSON_DEFINE_PROGMEM_ARRAY(const char*, messages,
                                     {s0, s1, s2, s3, s4, s5});
    return reinterpret_cast<const __FlashStringHelper*>(
        detail::pgm_read(messages + code_));
  }
#endif
 private:
  Code code_;
};
#if ARDUINOJSON_ENABLE_STD_STREAM
inline std::ostream& operator<<(std::ostream& s,
                                const DeserializationError& e) {
  s << e.c_str();
  return s;
}
inline std::ostream& operator<<(std::ostream& s, DeserializationError::Code c) {
  s << DeserializationError(c).c_str();
  return s;
}
#endif
namespace DeserializationOption {
class Filter {
 public:
  explicit Filter(JsonVariantConst v) : variant_(v) {}
  bool allow() const {
    return variant_;
  }
  bool allowArray() const {
    return variant_ == true || variant_.is<JsonArrayConst>();
  }
  bool allowObject() const {
    return variant_ == true || variant_.is<JsonObjectConst>();
  }
  bool allowValue() const {
    return variant_ == true;
  }
  template <typename TKey>
  Filter operator[](const TKey& key) const {
    if (variant_ == true)  // "true" means "allow recursively"
      return *this;
    JsonVariantConst member = variant_[key];
    return Filter(member.isNull() ? variant_["*"] : member);
  }
 private:
  JsonVariantConst variant_;
};
}  // namespace DeserializationOption
namespace detail {
struct AllowAllFilter {
  bool allow() const {
    return true;
  }
  bool allowArray() const {
    return true;
  }
  bool allowObject() const {
    return true;
  }
  bool allowValue() const {
    return true;
  }
  template <typename TKey>
  AllowAllFilter operator[](const TKey&) const {
    return AllowAllFilter();
  }
};
}  // namespace detail
namespace DeserializationOption {
class NestingLimit {
 public:
  NestingLimit() : value_(ARDUINOJSON_DEFAULT_NESTING_LIMIT) {}
  explicit NestingLimit(uint8_t n) : value_(n) {}
  NestingLimit decrement() const {
    ARDUINOJSON_ASSERT(value_ > 0);
    return NestingLimit(static_cast<uint8_t>(value_ - 1));
  }
  bool reached() const {
    return value_ == 0;
  }
 private:
  uint8_t value_;
};
}  // namespace DeserializationOption
ARDUINOJSON_END_PUBLIC_NAMESPACE
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <typename TFilter>
struct DeserializationOptions {
  TFilter filter;
  DeserializationOption::NestingLimit nestingLimit;
};
template <typename TFilter>
inline DeserializationOptions<TFilter> makeDeserializationOptions(
    TFilter filter, DeserializationOption::NestingLimit nestingLimit = {}) {
  return {filter, nestingLimit};
}
template <typename TFilter>
inline DeserializationOptions<TFilter> makeDeserializationOptions(
    DeserializationOption::NestingLimit nestingLimit, TFilter filter) {
  return {filter, nestingLimit};
}
inline DeserializationOptions<AllowAllFilter> makeDeserializationOptions(
    DeserializationOption::NestingLimit nestingLimit = {}) {
  return {{}, nestingLimit};
}
template <class T>
T&& forward(typename remove_reference<T>::type& t) noexcept {
  return static_cast<T&&>(t);
}
template <typename TSource, typename Enable = void>
struct Reader {
 public:
  Reader(TSource& source) : source_(&source) {}
  int read() {
    return source_->read();  // Error here? See https://arduinojson.org/v6/invalid-input/
  }
  size_t readBytes(char* buffer, size_t length) {
    return source_->readBytes(buffer, length);
  }
 private:
  TSource* source_;
};
template <typename TSource, typename Enable = void>
struct BoundedReader {
};
template <typename TIterator>
class IteratorReader {
  TIterator ptr_, end_;
 public:
  explicit IteratorReader(TIterator begin, TIterator end)
      : ptr_(begin), end_(end) {}
  int read() {
    if (ptr_ < end_)
      return static_cast<unsigned char>(*ptr_++);
    else
      return -1;
  }
  size_t readBytes(char* buffer, size_t length) {
    size_t i = 0;
    while (i < length && ptr_ < end_)
      buffer[i++] = *ptr_++;
    return i;
  }
};
template <typename T>
struct void_ {
  typedef void type;
};
template <typename TSource>
struct Reader<TSource, typename void_<typename TSource::const_iterator>::type>
    : IteratorReader<typename TSource::const_iterator> {
  explicit Reader(const TSource& source)
      : IteratorReader<typename TSource::const_iterator>(source.begin(),
                                                         source.end()) {}
};
template <typename T>
struct IsCharOrVoid {
  static const bool value =
      is_same<T, void>::value || is_same<T, char>::value ||
      is_same<T, unsigned char>::value || is_same<T, signed char>::value;
};
template <typename T>
struct IsCharOrVoid<const T> : IsCharOrVoid<T> {};
template <typename TSource>
struct Reader<TSource*,
              typename enable_if<IsCharOrVoid<TSource>::value>::type> {
  const char* ptr_;
 public:
  explicit Reader(const void* ptr)
      : ptr_(ptr ? reinterpret_cast<const char*>(ptr) : "") {}
  int read() {
    return static_cast<unsigned char>(*ptr_++);
  }
  size_t readBytes(char* buffer, size_t length) {
    for (size_t i = 0; i < length; i++)
      buffer[i] = *ptr_++;
    return length;
  }
};
template <typename TSource>
struct BoundedReader<TSource*,
                     typename enable_if<IsCharOrVoid<TSource>::value>::type>
    : public IteratorReader<const char*> {
 public:
  explicit BoundedReader(const void* ptr, size_t len)
      : IteratorReader<const char*>(reinterpret_cast<const char*>(ptr),
                                    reinterpret_cast<const char*>(ptr) + len) {}
};
template <typename TVariant>
struct Reader<TVariant, typename enable_if<IsVariant<TVariant>::value>::type>
    : Reader<char*, void> {
  explicit Reader(const TVariant& x)
      : Reader<char*, void>(x.template as<const char*>()) {}
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
#if ARDUINOJSON_ENABLE_ARDUINO_STREAM
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <typename TSource>
struct Reader<TSource,
              typename enable_if<is_base_of<Stream, TSource>::value>::type> {
 public:
  explicit Reader(Stream& stream) : stream_(&stream) {}
  int read() {
    char c;
    return stream_->readBytes(&c, 1) ? static_cast<unsigned char>(c) : -1;
  }
  size_t readBytes(char* buffer, size_t length) {
    return stream_->readBytes(buffer, length);
  }
 private:
  Stream* stream_;
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
#endif
#if ARDUINOJSON_ENABLE_ARDUINO_STRING
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <typename TSource>
struct Reader<TSource,
              typename enable_if<is_base_of<::String, TSource>::value>::type>
    : BoundedReader<const char*> {
  explicit Reader(const ::String& s)
      : BoundedReader<const char*>(s.c_str(), s.length()) {}
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
#endif
#if ARDUINOJSON_ENABLE_PROGMEM
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <>
struct Reader<const __FlashStringHelper*, void> {
  const char* ptr_;
 public:
  explicit Reader(const __FlashStringHelper* ptr)
      : ptr_(reinterpret_cast<const char*>(ptr)) {}
  int read() {
    return pgm_read_byte(ptr_++);
  }
  size_t readBytes(char* buffer, size_t length) {
    memcpy_P(buffer, ptr_, length);
    ptr_ += length;
    return length;
  }
};
template <>
struct BoundedReader<const __FlashStringHelper*, void> {
  const char* ptr_;
  const char* end_;
 public:
  explicit BoundedReader(const __FlashStringHelper* ptr, size_t size)
      : ptr_(reinterpret_cast<const char*>(ptr)), end_(ptr_ + size) {}
  int read() {
    if (ptr_ < end_)
      return pgm_read_byte(ptr_++);
    else
      return -1;
  }
  size_t readBytes(char* buffer, size_t length) {
    size_t available = static_cast<size_t>(end_ - ptr_);
    if (available < length)
      length = available;
    memcpy_P(buffer, ptr_, length);
    ptr_ += length;
    return length;
  }
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
#endif
#if ARDUINOJSON_ENABLE_STD_STREAM
#include <istream>
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <typename TSource>
struct Reader<TSource, typename enable_if<
                           is_base_of<std::istream, TSource>::value>::type> {
 public:
  explicit Reader(std::istream& stream) : stream_(&stream) {}
  int read() {
    return stream_->get();
  }
  size_t readBytes(char* buffer, size_t length) {
    stream_->read(buffer, static_cast<std::streamsize>(length));
    return static_cast<size_t>(stream_->gcount());
  }
 private:
  std::istream* stream_;
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
#endif
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <typename TInput>
Reader<typename remove_reference<TInput>::type> makeReader(TInput&& input) {
  return Reader<typename remove_reference<TInput>::type>{
      detail::forward<TInput>(input)};
}
template <typename TChar>
BoundedReader<TChar*> makeReader(TChar* input, size_t inputSize) {
  return BoundedReader<TChar*>{input, inputSize};
}
class StringCopier {
 public:
  StringCopier(MemoryPool* pool) : pool_(pool) {}
  void startString() {
    pool_->getFreeZone(&ptr_, &capacity_);
    size_ = 0;
    if (capacity_ == 0)
      pool_->markAsOverflowed();
  }
  JsonString save() {
    ARDUINOJSON_ASSERT(ptr_);
    ARDUINOJSON_ASSERT(size_ < capacity_);  // needs room for the terminator
    return JsonString(pool_->saveStringFromFreeZone(size_), size_,
                      JsonString::Copied);
  }
  void append(const char* s) {
    while (*s)
      append(*s++);
  }
  void append(const char* s, size_t n) {
    while (n-- > 0)
      append(*s++);
  }
  void append(char c) {
    if (size_ + 1 < capacity_)
      ptr_[size_++] = c;
    else
      pool_->markAsOverflowed();
  }
  bool isValid() const {
    return !pool_->overflowed();
  }
  size_t size() const {
    return size_;
  }
  JsonString str() const {
    ARDUINOJSON_ASSERT(ptr_);
    ARDUINOJSON_ASSERT(size_ < capacity_);
    ptr_[size_] = 0;
    return JsonString(ptr_, size_, JsonString::Copied);
  }
 private:
  MemoryPool* pool_;
  char* ptr_;
  size_t size_, capacity_;
};
class StringMover {
 public:
  StringMover(char* ptr) : writePtr_(ptr) {}
  void startString() {
    startPtr_ = writePtr_;
  }
  FORCE_INLINE JsonString save() {
    JsonString s = str();
    writePtr_++;
    return s;
  }
  void append(char c) {
    *writePtr_++ = c;
  }
  bool isValid() const {
    return true;
  }
  JsonString str() const {
    writePtr_[0] = 0;  // terminator
    return JsonString(startPtr_, size(), JsonString::Linked);
  }
  size_t size() const {
    return size_t(writePtr_ - startPtr_);
  }
 private:
  char* writePtr_;
  char* startPtr_;
};
template <typename TInput>
StringCopier makeStringStorage(TInput&, MemoryPool* pool) {
  ARDUINOJSON_ASSERT(pool != 0);
  return StringCopier(pool);
}
template <typename TChar>
StringMover makeStringStorage(
    TChar* input, MemoryPool*,
    typename enable_if<!is_const<TChar>::value>::type* = 0) {
  return StringMover(reinterpret_cast<char*>(input));
}
template <typename...>
struct first_or_void {
  using type = void;
};
template <typename T, typename... Rest>
struct first_or_void<T, Rest...> {
  using type = T;
};
template <template <typename, typename> class TDeserializer, typename TReader,
          typename TWriter>
TDeserializer<TReader, TWriter> makeDeserializer(MemoryPool* pool,
                                                 TReader reader,
                                                 TWriter writer) {
  ARDUINOJSON_ASSERT(pool != 0);
  return TDeserializer<TReader, TWriter>(pool, reader, writer);
}
template <template <typename, typename> class TDeserializer, typename TStream,
          typename... Args,
          typename = typename enable_if<  // issue #1897
              !is_integral<typename first_or_void<Args...>::type>::value>::type>
DeserializationError deserialize(JsonDocument& doc, TStream&& input,
                                 Args... args) {
  auto reader = makeReader(detail::forward<TStream>(input));
  auto data = VariantAttorney::getData(doc);
  auto pool = VariantAttorney::getPool(doc);
  auto options = makeDeserializationOptions(args...);
  doc.clear();
  return makeDeserializer<TDeserializer>(pool, reader,
                                         makeStringStorage(input, pool))
      .parse(*data, options.filter, options.nestingLimit);
}
template <template <typename, typename> class TDeserializer, typename TChar,
          typename Size, typename... Args,
          typename = typename enable_if<is_integral<Size>::value>::type>
DeserializationError deserialize(JsonDocument& doc, TChar* input,
                                 Size inputSize, Args... args) {
  auto reader = makeReader(input, size_t(inputSize));
  auto data = VariantAttorney::getData(doc);
  auto pool = VariantAttorney::getPool(doc);
  auto options = makeDeserializationOptions(args...);
  doc.clear();
  return makeDeserializer<TDeserializer>(pool, reader,
                                         makeStringStorage(input, pool))
      .parse(*data, options.filter, options.nestingLimit);
}
template <typename TReader>
class Latch {
 public:
  Latch(TReader reader) : reader_(reader), loaded_(false) {
#if ARDUINOJSON_DEBUG
    ended_ = false;
#endif
  }
  void clear() {
    loaded_ = false;
  }
  int last() const {
    return current_;
  }
  FORCE_INLINE char current() {
    if (!loaded_) {
      load();
    }
    return current_;
  }
 private:
  void load() {
    ARDUINOJSON_ASSERT(!ended_);
    int c = reader_.read();
#if ARDUINOJSON_DEBUG
    if (c <= 0)
      ended_ = true;
#endif
    current_ = static_cast<char>(c > 0 ? c : 0);
    loaded_ = true;
  }
  TReader reader_;
  char current_;  // NOLINT(clang-analyzer-optin.cplusplus.UninitializedObject)
  bool loaded_;
#if ARDUINOJSON_DEBUG
  bool ended_;
#endif
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
#if defined(__GNUC__)
#  if __GNUC__ >= 7
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#  endif
#endif
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
namespace Utf16 {
inline bool isHighSurrogate(uint16_t codeunit) {
  return codeunit >= 0xD800 && codeunit < 0xDC00;
}
inline bool isLowSurrogate(uint16_t codeunit) {
  return codeunit >= 0xDC00 && codeunit < 0xE000;
}
class Codepoint {
 public:
  Codepoint() : highSurrogate_(0), codepoint_(0) {}
  bool append(uint16_t codeunit) {
    if (isHighSurrogate(codeunit)) {
      highSurrogate_ = codeunit & 0x3FF;
      return false;
    }
    if (isLowSurrogate(codeunit)) {
      codepoint_ =
          uint32_t(0x10000 + ((highSurrogate_ << 10) | (codeunit & 0x3FF)));
      return true;
    }
    codepoint_ = codeunit;
    return true;
  }
  uint32_t value() const {
    return codepoint_;
  }
 private:
  uint16_t highSurrogate_;
  uint32_t codepoint_;
};
}  // namespace Utf16
ARDUINOJSON_END_PRIVATE_NAMESPACE
#if defined(__GNUC__)
#  if __GNUC__ >= 8
#    pragma GCC diagnostic pop
#  endif
#endif
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
namespace Utf8 {
template <typename TStringBuilder>
inline void encodeCodepoint(uint32_t codepoint32, TStringBuilder& str) {
  if (codepoint32 < 0x80) {
    str.append(char(codepoint32));
  } else {
    char buf[5];
    char* p = buf;
    *(p++) = 0;
    *(p++) = char((codepoint32 | 0x80) & 0xBF);
    uint16_t codepoint16 = uint16_t(codepoint32 >> 6);
    if (codepoint16 < 0x20) {  // 0x800
      *(p++) = char(codepoint16 | 0xC0);
    } else {
      *(p++) = char((codepoint16 | 0x80) & 0xBF);
      codepoint16 = uint16_t(codepoint16 >> 6);
      if (codepoint16 < 0x10) {  // 0x10000
        *(p++) = char(codepoint16 | 0xE0);
      } else {
        *(p++) = char((codepoint16 | 0x80) & 0xBF);
        codepoint16 = uint16_t(codepoint16 >> 6);
        *(p++) = char(codepoint16 | 0xF0);
      }
    }
    while (*(--p)) {
      str.append(*p);
    }
  }
}
}  // namespace Utf8
template <typename TReader, typename TStringStorage>
class JsonDeserializer {
 public:
  JsonDeserializer(MemoryPool* pool, TReader reader,
                   TStringStorage stringStorage)
      : stringStorage_(stringStorage),
        foundSomething_(false),
        latch_(reader),
        pool_(pool) {}
  template <typename TFilter>
  DeserializationError parse(VariantData& variant, TFilter filter,
                             DeserializationOption::NestingLimit nestingLimit) {
    DeserializationError::Code err;
    err = parseVariant(variant, filter, nestingLimit);
    if (!err && latch_.last() != 0 && !variant.isEnclosed()) {
      return DeserializationError::InvalidInput;
    }
    return err;
  }
 private:
  char current() {
    return latch_.current();
  }
  void move() {
    latch_.clear();
  }
  bool eat(char charToSkip) {
    if (current() != charToSkip)
      return false;
    move();
    return true;
  }
  template <typename TFilter>
  DeserializationError::Code parseVariant(
      VariantData& variant, TFilter filter,
      DeserializationOption::NestingLimit nestingLimit) {
    DeserializationError::Code err;
    err = skipSpacesAndComments();
    if (err)
      return err;
    switch (current()) {
      case '[':
        if (filter.allowArray())
          return parseArray(variant.toArray(), filter, nestingLimit);
        else
          return skipArray(nestingLimit);
      case '{':
        if (filter.allowObject())
          return parseObject(variant.toObject(), filter, nestingLimit);
        else
          return skipObject(nestingLimit);
      case '\"':
      case '\'':
        if (filter.allowValue())
          return parseStringValue(variant);
        else
          return skipQuotedString();
      case 't':
        if (filter.allowValue())
          variant.setBoolean(true);
        return skipKeyword("true");
      case 'f':
        if (filter.allowValue())
          variant.setBoolean(false);
        return skipKeyword("false");
      case 'n':
        return skipKeyword("null");
      default:
        if (filter.allowValue())
          return parseNumericValue(variant);
        else
          return skipNumericValue();
    }
  }
  DeserializationError::Code skipVariant(
      DeserializationOption::NestingLimit nestingLimit) {
    DeserializationError::Code err;
    err = skipSpacesAndComments();
    if (err)
      return err;
    switch (current()) {
      case '[':
        return skipArray(nestingLimit);
      case '{':
        return skipObject(nestingLimit);
      case '\"':
      case '\'':
        return skipQuotedString();
      case 't':
        return skipKeyword("true");
      case 'f':
        return skipKeyword("false");
      case 'n':
        return skipKeyword("null");
      default:
        return skipNumericValue();
    }
  }
  template <typename TFilter>
  DeserializationError::Code parseArray(
      CollectionData& array, TFilter filter,
      DeserializationOption::NestingLimit nestingLimit) {
    DeserializationError::Code err;
    if (nestingLimit.reached())
      return DeserializationError::TooDeep;
    ARDUINOJSON_ASSERT(current() == '[');
    move();
    err = skipSpacesAndComments();
    if (err)
      return err;
    if (eat(']'))
      return DeserializationError::Ok;
    TFilter memberFilter = filter[0UL];
    for (;;) {
      if (memberFilter.allow()) {
        VariantData* value = array.addElement(pool_);
        if (!value)
          return DeserializationError::NoMemory;
        err = parseVariant(*value, memberFilter, nestingLimit.decrement());
        if (err)
          return err;
      } else {
        err = skipVariant(nestingLimit.decrement());
        if (err)
          return err;
      }
      err = skipSpacesAndComments();
      if (err)
        return err;
      if (eat(']'))
        return DeserializationError::Ok;
      if (!eat(','))
        return DeserializationError::InvalidInput;
    }
  }
  DeserializationError::Code skipArray(
      DeserializationOption::NestingLimit nestingLimit) {
    DeserializationError::Code err;
    if (nestingLimit.reached())
      return DeserializationError::TooDeep;
    ARDUINOJSON_ASSERT(current() == '[');
    move();
    for (;;) {
      err = skipVariant(nestingLimit.decrement());
      if (err)
        return err;
      err = skipSpacesAndComments();
      if (err)
        return err;
      if (eat(']'))
        return DeserializationError::Ok;
      if (!eat(','))
        return DeserializationError::InvalidInput;
    }
  }
  template <typename TFilter>
  DeserializationError::Code parseObject(
      CollectionData& object, TFilter filter,
      DeserializationOption::NestingLimit nestingLimit) {
    DeserializationError::Code err;
    if (nestingLimit.reached())
      return DeserializationError::TooDeep;
    ARDUINOJSON_ASSERT(current() == '{');
    move();
    err = skipSpacesAndComments();
    if (err)
      return err;
    if (eat('}'))
      return DeserializationError::Ok;
    for (;;) {
      err = parseKey();
      if (err)
        return err;
      err = skipSpacesAndComments();
      if (err)
        return err;
      if (!eat(':'))
        return DeserializationError::InvalidInput;
      JsonString key = stringStorage_.str();
      TFilter memberFilter = filter[key.c_str()];
      if (memberFilter.allow()) {
        VariantData* variant = object.getMember(adaptString(key.c_str()));
        if (!variant) {
          key = stringStorage_.save();
          VariantSlot* slot = object.addSlot(pool_);
          if (!slot)
            return DeserializationError::NoMemory;
          slot->setKey(key);
          variant = slot->data();
        }
        err = parseVariant(*variant, memberFilter, nestingLimit.decrement());
        if (err)
          return err;
      } else {
        err = skipVariant(nestingLimit.decrement());
        if (err)
          return err;
      }
      err = skipSpacesAndComments();
      if (err)
        return err;
      if (eat('}'))
        return DeserializationError::Ok;
      if (!eat(','))
        return DeserializationError::InvalidInput;
      err = skipSpacesAndComments();
      if (err)
        return err;
    }
  }
  DeserializationError::Code skipObject(
      DeserializationOption::NestingLimit nestingLimit) {
    DeserializationError::Code err;
    if (nestingLimit.reached())
      return DeserializationError::TooDeep;
    ARDUINOJSON_ASSERT(current() == '{');
    move();
    err = skipSpacesAndComments();
    if (err)
      return err;
    if (eat('}'))
      return DeserializationError::Ok;
    for (;;) {
      err = skipKey();
      if (err)
        return err;
      err = skipSpacesAndComments();
      if (err)
        return err;
      if (!eat(':'))
        return DeserializationError::InvalidInput;
      err = skipVariant(nestingLimit.decrement());
      if (err)
        return err;
      err = skipSpacesAndComments();
      if (err)
        return err;
      if (eat('}'))
        return DeserializationError::Ok;
      if (!eat(','))
        return DeserializationError::InvalidInput;
      err = skipSpacesAndComments();
      if (err)
        return err;
    }
  }
  DeserializationError::Code parseKey() {
    stringStorage_.startString();
    if (isQuote(current())) {
      return parseQuotedString();
    } else {
      return parseNonQuotedString();
    }
  }
  DeserializationError::Code parseStringValue(VariantData& variant) {
    DeserializationError::Code err;
    stringStorage_.startString();
    err = parseQuotedString();
    if (err)
      return err;
    variant.setString(stringStorage_.save());
    return DeserializationError::Ok;
  }
  DeserializationError::Code parseQuotedString() {
#if ARDUINOJSON_DECODE_UNICODE
    Utf16::Codepoint codepoint;
    DeserializationError::Code err;
#endif
    const char stopChar = current();
    move();
    for (;;) {
      char c = current();
      move();
      if (c == stopChar)
        break;
      if (c == '\0')
        return DeserializationError::IncompleteInput;
      if (c == '\\') {
        c = current();
        if (c == '\0')
          return DeserializationError::IncompleteInput;
        if (c == 'u') {
#if ARDUINOJSON_DECODE_UNICODE
          move();
          uint16_t codeunit;
          err = parseHex4(codeunit);
          if (err)
            return err;
          if (codepoint.append(codeunit))
            Utf8::encodeCodepoint(codepoint.value(), stringStorage_);
#else
          stringStorage_.append('\\');
#endif
          continue;
        }
        c = EscapeSequence::unescapeChar(c);
        if (c == '\0')
          return DeserializationError::InvalidInput;
        move();
      }
      stringStorage_.append(c);
    }
    if (!stringStorage_.isValid())
      return DeserializationError::NoMemory;
    return DeserializationError::Ok;
  }
  DeserializationError::Code parseNonQuotedString() {
    char c = current();
    ARDUINOJSON_ASSERT(c);
    if (canBeInNonQuotedString(c)) {  // no quotes
      do {
        move();
        stringStorage_.append(c);
        c = current();
      } while (canBeInNonQuotedString(c));
    } else {
      return DeserializationError::InvalidInput;
    }
    if (!stringStorage_.isValid())
      return DeserializationError::NoMemory;
    return DeserializationError::Ok;
  }
  DeserializationError::Code skipKey() {
    if (isQuote(current())) {
      return skipQuotedString();
    } else {
      return skipNonQuotedString();
    }
  }
  DeserializationError::Code skipQuotedString() {
    const char stopChar = current();
    move();
    for (;;) {
      char c = current();
      move();
      if (c == stopChar)
        break;
      if (c == '\0')
        return DeserializationError::IncompleteInput;
      if (c == '\\') {
        if (current() != '\0')
          move();
      }
    }
    return DeserializationError::Ok;
  }
  DeserializationError::Code skipNonQuotedString() {
    char c = current();
    while (canBeInNonQuotedString(c)) {
      move();
      c = current();
    }
    return DeserializationError::Ok;
  }
  DeserializationError::Code parseNumericValue(VariantData& result) {
    uint8_t n = 0;
    char c = current();
    while (canBeInNumber(c) && n < 63) {
      move();
      buffer_[n++] = c;
      c = current();
    }
    buffer_[n] = 0;
    if (!parseNumber(buffer_, result))
      return DeserializationError::InvalidInput;
    return DeserializationError::Ok;
  }
  DeserializationError::Code skipNumericValue() {
    char c = current();
    while (canBeInNumber(c)) {
      move();
      c = current();
    }
    return DeserializationError::Ok;
  }
  DeserializationError::Code parseHex4(uint16_t& result) {
    result = 0;
    for (uint8_t i = 0; i < 4; ++i) {
      char digit = current();
      if (!digit)
        return DeserializationError::IncompleteInput;
      uint8_t value = decodeHex(digit);
      if (value > 0x0F)
        return DeserializationError::InvalidInput;
      result = uint16_t((result << 4) | value);
      move();
    }
    return DeserializationError::Ok;
  }
  static inline bool isBetween(char c, char min, char max) {
    return min <= c && c <= max;
  }
  static inline bool canBeInNumber(char c) {
    return isBetween(c, '0', '9') || c == '+' || c == '-' || c == '.' ||
#if ARDUINOJSON_ENABLE_NAN || ARDUINOJSON_ENABLE_INFINITY
           isBetween(c, 'A', 'Z') || isBetween(c, 'a', 'z');
#else
           c == 'e' || c == 'E';
#endif
  }
  static inline bool canBeInNonQuotedString(char c) {
    return isBetween(c, '0', '9') || isBetween(c, '_', 'z') ||
           isBetween(c, 'A', 'Z');
  }
  static inline bool isQuote(char c) {
    return c == '\'' || c == '\"';
  }
  static inline uint8_t decodeHex(char c) {
    if (c < 'A')
      return uint8_t(c - '0');
    c = char(c & ~0x20);  // uppercase
    return uint8_t(c - 'A' + 10);
  }
  DeserializationError::Code skipSpacesAndComments() {
    for (;;) {
      switch (current()) {
        case '\0':
          return foundSomething_ ? DeserializationError::IncompleteInput
                                 : DeserializationError::EmptyInput;
        case ' ':
        case '\t':
        case '\r':
        case '\n':
          move();
          continue;
#if ARDUINOJSON_ENABLE_COMMENTS
        case '/':
          move();  // skip '/'
          switch (current()) {
            case '*': {
              move();  // skip '*'
              bool wasStar = false;
              for (;;) {
                char c = current();
                if (c == '\0')
                  return DeserializationError::IncompleteInput;
                if (c == '/' && wasStar) {
                  move();
                  break;
                }
                wasStar = c == '*';
                move();
              }
              break;
            }
            case '/':
              for (;;) {
                move();
                char c = current();
                if (c == '\0')
                  return DeserializationError::IncompleteInput;
                if (c == '\n')
                  break;
              }
              break;
            default:
              return DeserializationError::InvalidInput;
          }
          break;
#endif
        default:
          foundSomething_ = true;
          return DeserializationError::Ok;
      }
    }
  }
  DeserializationError::Code skipKeyword(const char* s) {
    while (*s) {
      char c = current();
      if (c == '\0')
        return DeserializationError::IncompleteInput;
      if (*s != c)
        return DeserializationError::InvalidInput;
      ++s;
      move();
    }
    return DeserializationError::Ok;
  }
  TStringStorage stringStorage_;
  bool foundSomething_;
  Latch<TReader> latch_;
  MemoryPool* pool_;
  char buffer_[64];  // using a member instead of a local variable because it
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
ARDUINOJSON_BEGIN_PUBLIC_NAMESPACE
template <typename... Args>
DeserializationError deserializeJson(JsonDocument& doc, Args&&... args) {
  using namespace detail;
  return deserialize<JsonDeserializer>(doc, detail::forward<Args>(args)...);
}
template <typename TChar, typename... Args>
DeserializationError deserializeJson(JsonDocument& doc, TChar* input,
                                     Args&&... args) {
  using namespace detail;
  return deserialize<JsonDeserializer>(doc, input,
                                       detail::forward<Args>(args)...);
}
ARDUINOJSON_END_PUBLIC_NAMESPACE
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <typename TWriter>
class PrettyJsonSerializer : public JsonSerializer<TWriter> {
  typedef JsonSerializer<TWriter> base;
 public:
  PrettyJsonSerializer(TWriter writer) : base(writer), nesting_(0) {}
  size_t visitArray(const CollectionData& array) {
    const VariantSlot* slot = array.head();
    if (slot) {
      base::write("[\r\n");
      nesting_++;
      while (slot != 0) {
        indent();
        slot->data()->accept(*this);
        slot = slot->next();
        base::write(slot ? ",\r\n" : "\r\n");
      }
      nesting_--;
      indent();
      base::write("]");
    } else {
      base::write("[]");
    }
    return this->bytesWritten();
  }
  size_t visitObject(const CollectionData& object) {
    const VariantSlot* slot = object.head();
    if (slot) {
      base::write("{\r\n");
      nesting_++;
      while (slot != 0) {
        indent();
        base::visitString(slot->key());
        base::write(": ");
        slot->data()->accept(*this);
        slot = slot->next();
        base::write(slot ? ",\r\n" : "\r\n");
      }
      nesting_--;
      indent();
      base::write("}");
    } else {
      base::write("{}");
    }
    return this->bytesWritten();
  }
 private:
  void indent() {
    for (uint8_t i = 0; i < nesting_; i++)
      base::write(ARDUINOJSON_TAB);
  }
  uint8_t nesting_;
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
ARDUINOJSON_BEGIN_PUBLIC_NAMESPACE
template <typename TDestination>
size_t serializeJsonPretty(JsonVariantConst source, TDestination& destination) {
  using namespace ArduinoJson::detail;
  return serialize<PrettyJsonSerializer>(source, destination);
}
inline size_t serializeJsonPretty(JsonVariantConst source, void* buffer,
                                  size_t bufferSize) {
  using namespace ArduinoJson::detail;
  return serialize<PrettyJsonSerializer>(source, buffer, bufferSize);
}
inline size_t measureJsonPretty(JsonVariantConst source) {
  using namespace ArduinoJson::detail;
  return measure<PrettyJsonSerializer>(source);
}
ARDUINOJSON_END_PUBLIC_NAMESPACE
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
#if ARDUINOJSON_LITTLE_ENDIAN
inline void swapBytes(uint8_t& a, uint8_t& b) {
  uint8_t t(a);
  a = b;
  b = t;
}
inline void fixEndianess(uint8_t* p, integral_constant<size_t, 8>) {
  swapBytes(p[0], p[7]);
  swapBytes(p[1], p[6]);
  swapBytes(p[2], p[5]);
  swapBytes(p[3], p[4]);
}
inline void fixEndianess(uint8_t* p, integral_constant<size_t, 4>) {
  swapBytes(p[0], p[3]);
  swapBytes(p[1], p[2]);
}
inline void fixEndianess(uint8_t* p, integral_constant<size_t, 2>) {
  swapBytes(p[0], p[1]);
}
inline void fixEndianess(uint8_t*, integral_constant<size_t, 1>) {}
template <typename T>
inline void fixEndianess(T& value) {
  fixEndianess(reinterpret_cast<uint8_t*>(&value),
               integral_constant<size_t, sizeof(T)>());
}
#else
template <typename T>
inline void fixEndianess(T&) {}
#endif
inline void doubleToFloat(const uint8_t d[8], uint8_t f[4]) {
  f[0] = uint8_t((d[0] & 0xC0) | (d[0] << 3 & 0x3f) | (d[1] >> 5));
  f[1] = uint8_t((d[1] << 3) | (d[2] >> 5));
  f[2] = uint8_t((d[2] << 3) | (d[3] >> 5));
  f[3] = uint8_t((d[3] << 3) | (d[4] >> 5));
}
template <typename TReader, typename TStringStorage>
class MsgPackDeserializer {
 public:
  MsgPackDeserializer(MemoryPool* pool, TReader reader,
                      TStringStorage stringStorage)
      : pool_(pool),
        reader_(reader),
        stringStorage_(stringStorage),
        foundSomething_(false) {}
  template <typename TFilter>
  DeserializationError parse(VariantData& variant, TFilter filter,
                             DeserializationOption::NestingLimit nestingLimit) {
    DeserializationError::Code err;
    err = parseVariant(&variant, filter, nestingLimit);
    return foundSomething_ ? err : DeserializationError::EmptyInput;
  }
 private:
  template <typename TFilter>
  DeserializationError::Code parseVariant(
      VariantData* variant, TFilter filter,
      DeserializationOption::NestingLimit nestingLimit) {
    DeserializationError::Code err;
    uint8_t code = 0;  // TODO: why do we need to initialize this variable?
    err = readByte(code);
    if (err)
      return err;
    foundSomething_ = true;
    bool allowValue = filter.allowValue();
    if (allowValue) {
      ARDUINOJSON_ASSERT(variant != 0);
    }
    switch (code) {
      case 0xc0:
        return DeserializationError::Ok;
      case 0xc1:
        return DeserializationError::InvalidInput;
      case 0xc2:
        if (allowValue)
          variant->setBoolean(false);
        return DeserializationError::Ok;
      case 0xc3:
        if (allowValue)
          variant->setBoolean(true);
        return DeserializationError::Ok;
      case 0xc4:  // bin 8 (not supported)
        return skipString<uint8_t>();
      case 0xc5:  // bin 16 (not supported)
        return skipString<uint16_t>();
      case 0xc6:  // bin 32 (not supported)
        return skipString<uint32_t>();
      case 0xc7:  // ext 8 (not supported)
        return skipExt<uint8_t>();
      case 0xc8:  // ext 16 (not supported)
        return skipExt<uint16_t>();
      case 0xc9:  // ext 32 (not supported)
        return skipExt<uint32_t>();
      case 0xca:
        if (allowValue)
          return readFloat<float>(variant);
        else
          return skipBytes(4);
      case 0xcb:
        if (allowValue)
          return readDouble<double>(variant);
        else
          return skipBytes(8);
      case 0xcc:
        if (allowValue)
          return readInteger<uint8_t>(variant);
        else
          return skipBytes(1);
      case 0xcd:
        if (allowValue)
          return readInteger<uint16_t>(variant);
        else
          return skipBytes(2);
      case 0xce:
        if (allowValue)
          return readInteger<uint32_t>(variant);
        else
          return skipBytes(4);
      case 0xcf:
#if ARDUINOJSON_USE_LONG_LONG
        if (allowValue)
          return readInteger<uint64_t>(variant);
        else
          return skipBytes(8);
#else
        return skipBytes(8);  // not supported
#endif
      case 0xd0:
        if (allowValue)
          return readInteger<int8_t>(variant);
        else
          return skipBytes(1);
      case 0xd1:
        if (allowValue)
          return readInteger<int16_t>(variant);
        else
          return skipBytes(2);
      case 0xd2:
        if (allowValue)
          return readInteger<int32_t>(variant);
        else
          return skipBytes(4);
      case 0xd3:
#if ARDUINOJSON_USE_LONG_LONG
        if (allowValue)
          return readInteger<int64_t>(variant);
        else
          return skipBytes(8);  // not supported
#else
        return skipBytes(8);
#endif
      case 0xd4:  // fixext 1 (not supported)
        return skipBytes(2);
      case 0xd5:  // fixext 2 (not supported)
        return skipBytes(3);
      case 0xd6:  // fixext 4 (not supported)
        return skipBytes(5);
      case 0xd7:  // fixext 8 (not supported)
        return skipBytes(9);
      case 0xd8:  // fixext 16 (not supported)
        return skipBytes(17);
      case 0xd9:
        if (allowValue)
          return readString<uint8_t>(variant);
        else
          return skipString<uint8_t>();
      case 0xda:
        if (allowValue)
          return readString<uint16_t>(variant);
        else
          return skipString<uint16_t>();
      case 0xdb:
        if (allowValue)
          return readString<uint32_t>(variant);
        else
          return skipString<uint32_t>();
      case 0xdc:
        return readArray<uint16_t>(variant, filter, nestingLimit);
      case 0xdd:
        return readArray<uint32_t>(variant, filter, nestingLimit);
      case 0xde:
        return readObject<uint16_t>(variant, filter, nestingLimit);
      case 0xdf:
        return readObject<uint32_t>(variant, filter, nestingLimit);
    }
    switch (code & 0xf0) {
      case 0x80:
        return readObject(variant, code & 0x0F, filter, nestingLimit);
      case 0x90:
        return readArray(variant, code & 0x0F, filter, nestingLimit);
    }
    if ((code & 0xe0) == 0xa0) {
      if (allowValue)
        return readString(variant, code & 0x1f);
      else
        return skipBytes(code & 0x1f);
    }
    if (allowValue)
      variant->setInteger(static_cast<int8_t>(code));
    return DeserializationError::Ok;
  }
  DeserializationError::Code readByte(uint8_t& value) {
    int c = reader_.read();
    if (c < 0)
      return DeserializationError::IncompleteInput;
    value = static_cast<uint8_t>(c);
    return DeserializationError::Ok;
  }
  DeserializationError::Code readBytes(uint8_t* p, size_t n) {
    if (reader_.readBytes(reinterpret_cast<char*>(p), n) == n)
      return DeserializationError::Ok;
    return DeserializationError::IncompleteInput;
  }
  template <typename T>
  DeserializationError::Code readBytes(T& value) {
    return readBytes(reinterpret_cast<uint8_t*>(&value), sizeof(value));
  }
  DeserializationError::Code skipBytes(size_t n) {
    for (; n; --n) {
      if (reader_.read() < 0)
        return DeserializationError::IncompleteInput;
    }
    return DeserializationError::Ok;
  }
  template <typename T>
  DeserializationError::Code readInteger(T& value) {
    DeserializationError::Code err;
    err = readBytes(value);
    if (err)
      return err;
    fixEndianess(value);
    return DeserializationError::Ok;
  }
  template <typename T>
  DeserializationError::Code readInteger(VariantData* variant) {
    DeserializationError::Code err;
    T value;
    err = readInteger(value);
    if (err)
      return err;
    variant->setInteger(value);
    return DeserializationError::Ok;
  }
  template <typename T>
  typename enable_if<sizeof(T) == 4, DeserializationError::Code>::type
  readFloat(VariantData* variant) {
    DeserializationError::Code err;
    T value;
    err = readBytes(value);
    if (err)
      return err;
    fixEndianess(value);
    variant->setFloat(value);
    return DeserializationError::Ok;
  }
  template <typename T>
  typename enable_if<sizeof(T) == 8, DeserializationError::Code>::type
  readDouble(VariantData* variant) {
    DeserializationError::Code err;
    T value;
    err = readBytes(value);
    if (err)
      return err;
    fixEndianess(value);
    variant->setFloat(value);
    return DeserializationError::Ok;
  }
  template <typename T>
  typename enable_if<sizeof(T) == 4, DeserializationError::Code>::type
  readDouble(VariantData* variant) {
    DeserializationError::Code err;
    uint8_t i[8];  // input is 8 bytes
    T value;       // output is 4 bytes
    uint8_t* o = reinterpret_cast<uint8_t*>(&value);
    err = readBytes(i, 8);
    if (err)
      return err;
    doubleToFloat(i, o);
    fixEndianess(value);
    variant->setFloat(value);
    return DeserializationError::Ok;
  }
  template <typename T>
  DeserializationError::Code readString(VariantData* variant) {
    DeserializationError::Code err;
    T size;
    err = readInteger(size);
    if (err)
      return err;
    return readString(variant, size);
  }
  template <typename T>
  DeserializationError::Code readString() {
    DeserializationError::Code err;
    T size;
    err = readInteger(size);
    if (err)
      return err;
    return readString(size);
  }
  template <typename T>
  DeserializationError::Code skipString() {
    DeserializationError::Code err;
    T size;
    err = readInteger(size);
    if (err)
      return err;
    return skipBytes(size);
  }
  DeserializationError::Code readString(VariantData* variant, size_t n) {
    DeserializationError::Code err;
    err = readString(n);
    if (err)
      return err;
    variant->setString(stringStorage_.save());
    return DeserializationError::Ok;
  }
  DeserializationError::Code readString(size_t n) {
    DeserializationError::Code err;
    stringStorage_.startString();
    for (; n; --n) {
      uint8_t c;
      err = readBytes(c);
      if (err)
        return err;
      stringStorage_.append(static_cast<char>(c));
    }
    if (!stringStorage_.isValid())
      return DeserializationError::NoMemory;
    return DeserializationError::Ok;
  }
  template <typename TSize, typename TFilter>
  DeserializationError::Code readArray(
      VariantData* variant, TFilter filter,
      DeserializationOption::NestingLimit nestingLimit) {
    DeserializationError::Code err;
    TSize size;
    err = readInteger(size);
    if (err)
      return err;
    return readArray(variant, size, filter, nestingLimit);
  }
  template <typename TFilter>
  DeserializationError::Code readArray(
      VariantData* variant, size_t n, TFilter filter,
      DeserializationOption::NestingLimit nestingLimit) {
    DeserializationError::Code err;
    if (nestingLimit.reached())
      return DeserializationError::TooDeep;
    bool allowArray = filter.allowArray();
    CollectionData* array;
    if (allowArray) {
      ARDUINOJSON_ASSERT(variant != 0);
      array = &variant->toArray();
    } else {
      array = 0;
    }
    TFilter memberFilter = filter[0U];
    for (; n; --n) {
      VariantData* value;
      if (memberFilter.allow()) {
        ARDUINOJSON_ASSERT(array != 0);
        value = array->addElement(pool_);
        if (!value)
          return DeserializationError::NoMemory;
      } else {
        value = 0;
      }
      err = parseVariant(value, memberFilter, nestingLimit.decrement());
      if (err)
        return err;
    }
    return DeserializationError::Ok;
  }
  template <typename TSize, typename TFilter>
  DeserializationError::Code readObject(
      VariantData* variant, TFilter filter,
      DeserializationOption::NestingLimit nestingLimit) {
    DeserializationError::Code err;
    TSize size;
    err = readInteger(size);
    if (err)
      return err;
    return readObject(variant, size, filter, nestingLimit);
  }
  template <typename TFilter>
  DeserializationError::Code readObject(
      VariantData* variant, size_t n, TFilter filter,
      DeserializationOption::NestingLimit nestingLimit) {
    DeserializationError::Code err;
    if (nestingLimit.reached())
      return DeserializationError::TooDeep;
    CollectionData* object;
    if (filter.allowObject()) {
      ARDUINOJSON_ASSERT(variant != 0);
      object = &variant->toObject();
    } else {
      object = 0;
    }
    for (; n; --n) {
      err = readKey();
      if (err)
        return err;
      JsonString key = stringStorage_.str();
      TFilter memberFilter = filter[key.c_str()];
      VariantData* member;
      if (memberFilter.allow()) {
        ARDUINOJSON_ASSERT(object != 0);
        key = stringStorage_.save();
        VariantSlot* slot = object->addSlot(pool_);
        if (!slot)
          return DeserializationError::NoMemory;
        slot->setKey(key);
        member = slot->data();
      } else {
        member = 0;
      }
      err = parseVariant(member, memberFilter, nestingLimit.decrement());
      if (err)
        return err;
    }
    return DeserializationError::Ok;
  }
  DeserializationError::Code readKey() {
    DeserializationError::Code err;
    uint8_t code;
    err = readByte(code);
    if (err)
      return err;
    if ((code & 0xe0) == 0xa0)
      return readString(code & 0x1f);
    switch (code) {
      case 0xd9:
        return readString<uint8_t>();
      case 0xda:
        return readString<uint16_t>();
      case 0xdb:
        return readString<uint32_t>();
      default:
        return DeserializationError::InvalidInput;
    }
  }
  template <typename T>
  DeserializationError::Code skipExt() {
    DeserializationError::Code err;
    T size;
    err = readInteger(size);
    if (err)
      return err;
    return skipBytes(size + 1U);
  }
  MemoryPool* pool_;
  TReader reader_;
  TStringStorage stringStorage_;
  bool foundSomething_;
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
ARDUINOJSON_BEGIN_PUBLIC_NAMESPACE
template <typename... Args>
DeserializationError deserializeMsgPack(JsonDocument& doc, Args&&... args) {
  using namespace detail;
  return deserialize<MsgPackDeserializer>(doc, detail::forward<Args>(args)...);
}
template <typename TChar, typename... Args>
DeserializationError deserializeMsgPack(JsonDocument& doc, TChar* input,
                                        Args&&... args) {
  using namespace detail;
  return deserialize<MsgPackDeserializer>(doc, input,
                                          detail::forward<Args>(args)...);
}
ARDUINOJSON_END_PUBLIC_NAMESPACE
ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE
template <typename TWriter>
class MsgPackSerializer : public Visitor<size_t> {
 public:
  static const bool producesText = false;
  MsgPackSerializer(TWriter writer) : writer_(writer) {}
  template <typename T>
  typename enable_if<sizeof(T) == 4, size_t>::type visitFloat(T value32) {
    if (canConvertNumber<JsonInteger>(value32)) {
      JsonInteger truncatedValue = JsonInteger(value32);
      if (value32 == T(truncatedValue))
        return visitSignedInteger(truncatedValue);
    }
    writeByte(0xCA);
    writeInteger(value32);
    return bytesWritten();
  }
  template <typename T>
  ARDUINOJSON_NO_SANITIZE("float-cast-overflow")
  typename enable_if<sizeof(T) == 8, size_t>::type visitFloat(T value64) {
    float value32 = float(value64);
    if (value32 == value64)
      return visitFloat(value32);
    writeByte(0xCB);
    writeInteger(value64);
    return bytesWritten();
  }
  size_t visitArray(const CollectionData& array) {
    size_t n = array.size();
    if (n < 0x10) {
      writeByte(uint8_t(0x90 + n));
    } else if (n < 0x10000) {
      writeByte(0xDC);
      writeInteger(uint16_t(n));
    } else {
      writeByte(0xDD);
      writeInteger(uint32_t(n));
    }
    for (const VariantSlot* slot = array.head(); slot; slot = slot->next()) {
      slot->data()->accept(*this);
    }
    return bytesWritten();
  }
  size_t visitObject(const CollectionData& object) {
    size_t n = object.size();
    if (n < 0x10) {
      writeByte(uint8_t(0x80 + n));
    } else if (n < 0x10000) {
      writeByte(0xDE);
      writeInteger(uint16_t(n));
    } else {
      writeByte(0xDF);
      writeInteger(uint32_t(n));
    }
    for (const VariantSlot* slot = object.head(); slot; slot = slot->next()) {
      visitString(slot->key());
      slot->data()->accept(*this);
    }
    return bytesWritten();
  }
  size_t visitString(const char* value) {
    return visitString(value, strlen(value));
  }
  size_t visitString(const char* value, size_t n) {
    ARDUINOJSON_ASSERT(value != NULL);
    if (n < 0x20) {
      writeByte(uint8_t(0xA0 + n));
    } else if (n < 0x100) {
      writeByte(0xD9);
      writeInteger(uint8_t(n));
    } else if (n < 0x10000) {
      writeByte(0xDA);
      writeInteger(uint16_t(n));
    } else {
      writeByte(0xDB);
      writeInteger(uint32_t(n));
    }
    writeBytes(reinterpret_cast<const uint8_t*>(value), n);
    return bytesWritten();
  }
  size_t visitRawJson(const char* data, size_t size) {
    writeBytes(reinterpret_cast<const uint8_t*>(data), size);
    return bytesWritten();
  }
  size_t visitSignedInteger(JsonInteger value) {
    if (value > 0) {
      visitUnsignedInteger(static_cast<JsonUInt>(value));
    } else if (value >= -0x20) {
      writeInteger(int8_t(value));
    } else if (value >= -0x80) {
      writeByte(0xD0);
      writeInteger(int8_t(value));
    } else if (value >= -0x8000) {
      writeByte(0xD1);
      writeInteger(int16_t(value));
    }
#if ARDUINOJSON_USE_LONG_LONG
    else if (value >= -0x80000000LL)
#else
    else
#endif
    {
      writeByte(0xD2);
      writeInteger(int32_t(value));
    }
#if ARDUINOJSON_USE_LONG_LONG
    else {
      writeByte(0xD3);
      writeInteger(int64_t(value));
    }
#endif
    return bytesWritten();
  }
  size_t visitUnsignedInteger(JsonUInt value) {
    if (value <= 0x7F) {
      writeInteger(uint8_t(value));
    } else if (value <= 0xFF) {
      writeByte(0xCC);
      writeInteger(uint8_t(value));
    } else if (value <= 0xFFFF) {
      writeByte(0xCD);
      writeInteger(uint16_t(value));
    }
#if ARDUINOJSON_USE_LONG_LONG
    else if (value <= 0xFFFFFFFF)
#else
    else
#endif
    {
      writeByte(0xCE);
      writeInteger(uint32_t(value));
    }
#if ARDUINOJSON_USE_LONG_LONG
    else {
      writeByte(0xCF);
      writeInteger(uint64_t(value));
    }
#endif
    return bytesWritten();
  }
  size_t visitBoolean(bool value) {
    writeByte(value ? 0xC3 : 0xC2);
    return bytesWritten();
  }
  size_t visitNull() {
    writeByte(0xC0);
    return bytesWritten();
  }
 private:
  size_t bytesWritten() const {
    return writer_.count();
  }
  void writeByte(uint8_t c) {
    writer_.write(c);
  }
  void writeBytes(const uint8_t* p, size_t n) {
    writer_.write(p, n);
  }
  template <typename T>
  void writeInteger(T value) {
    fixEndianess(value);
    writeBytes(reinterpret_cast<uint8_t*>(&value), sizeof(value));
  }
  CountingDecorator<TWriter> writer_;
};
ARDUINOJSON_END_PRIVATE_NAMESPACE
ARDUINOJSON_BEGIN_PUBLIC_NAMESPACE
template <typename TDestination>
inline size_t serializeMsgPack(JsonVariantConst source, TDestination& output) {
  using namespace ArduinoJson::detail;
  return serialize<MsgPackSerializer>(source, output);
}
inline size_t serializeMsgPack(JsonVariantConst source, void* output,
                               size_t size) {
  using namespace ArduinoJson::detail;
  return serialize<MsgPackSerializer>(source, output, size);
}
inline size_t measureMsgPack(JsonVariantConst source) {
  using namespace ArduinoJson::detail;
  return measure<MsgPackSerializer>(source);
}
ARDUINOJSON_END_PUBLIC_NAMESPACE
#ifdef __GNUC__
#define ARDUINOJSON_PRAGMA(x) _Pragma(#x)
#define ARDUINOJSON_COMPILE_ERROR(msg) ARDUINOJSON_PRAGMA(GCC error msg)
#define ARDUINOJSON_STRINGIFY(S) #S
#define ARDUINOJSON_DEPRECATION_ERROR(X, Y) \
  ARDUINOJSON_COMPILE_ERROR(ARDUINOJSON_STRINGIFY(X is a Y from ArduinoJson 5. Please see https:/\/arduinojson.org/upgrade to learn how to upgrade your program to ArduinoJson version 6))
#define StaticJsonBuffer ARDUINOJSON_DEPRECATION_ERROR(StaticJsonBuffer, class)
#define DynamicJsonBuffer ARDUINOJSON_DEPRECATION_ERROR(DynamicJsonBuffer, class)
#define JsonBuffer ARDUINOJSON_DEPRECATION_ERROR(JsonBuffer, class)
#define RawJson ARDUINOJSON_DEPRECATION_ERROR(RawJson, function)
#define ARDUINOJSON_NAMESPACE _Pragma ("GCC warning \"ARDUINOJSON_NAMESPACE is deprecated, use ArduinoJson instead\"") ArduinoJson
#endif

using namespace ArduinoJson;

#else

#error ArduinoJson requires a C++ compiler, please change file extension to .cc or .cpp

#endif
