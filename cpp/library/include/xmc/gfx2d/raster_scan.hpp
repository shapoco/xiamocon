#ifndef XMC_GFX_RASTER_SCANNER_HPP
#define XMC_GFX_RASTER_SCANNER_HPP

#include "xmc/gfx2d/gfx_common.hpp"

namespace xmc {

class RasterScanGray1 {
 public:
  uint8_t *ptr;
  uint8_t mask;
  uint8_t temp;
  XMC_INLINE RasterScanGray1(uint8_t *data, int x)
      : ptr(data), mask(0x80 >> (x % 8)), temp(data[x / 8]) {}
  XMC_INLINE ~RasterScanGray1() { flush(); }
  XMC_INLINE uint8_t peek() const { return temp & mask; }
  XMC_INLINE uint8_t pop() {
    uint8_t result = peek();
    skip();
    return result;
  }
  XMC_INLINE void pushGray1(uint8_t value, const TextRenderArgs &args) {
    bool write = false;
    bool set = false;
    if (value && hasFlag(args.flags, TextRenderFlags::USE_FORE_COLOR)) {
      write = true;
      set = !!args.foreColor;
    } else if (hasFlag(args.flags, TextRenderFlags::USE_BACK_COLOR)) {
      write = true;
      set = !!args.backColor;
    }
    if (write) {
      if (set) {
        temp |= mask;
      } else {
        temp &= ~mask;
      }
    }
    skip();
  }
  XMC_INLINE void push444(uint16_t color) {
    if (convert444ToGray1(color)) {
      temp |= mask;
    } else {
      temp &= ~mask;
    }
    skip();
  }
  XMC_INLINE void push565(uint16_t color) {
    if (convert565ToGray1(color)) {
      temp |= mask;
    } else {
      temp &= ~mask;
    }
    skip();
  }
  XMC_INLINE void push4444(uint16_t color) {
    uint16_t a1 = color & 0xF000;
    if (a1 >= 0x8000) {
      push444(color);
    } else if (a1 == 0) {
      skip();
    }
  }
  XMC_INLINE void skip(int n = 1) {
    if (mask == 0x01) {
      *(ptr++) = temp;
    }
    for (int i = 0; i < n; i++) {
      mask = (mask >> 1) | ((mask & 1) << 7);
    }
    if (mask == 0x80) {
      temp = *ptr;
    }
  }
  XMC_INLINE void flush() { *ptr = temp; }
};

class RasterScan444 {
 public:
  int x;
  uint8_t *ptr;
  XMC_INLINE RasterScan444(uint8_t *data, int x)
      : x(x), ptr(data + x * 3 / 2) {}
  XMC_INLINE ~RasterScan444() { flush(); }
  XMC_INLINE uint16_t peek() const {
    if ((x & 1) == 0) {
      return ((uint16_t)(*ptr) << 4) | ((*ptr >> 4) & 0x0F);
    } else {
      return ((uint16_t)(*ptr & 0x0F) << 8) | (*(ptr + 1));
    }
  }
  XMC_INLINE uint16_t pop() {
    uint16_t value = peek();
    skip();
    return value;
  }
  XMC_INLINE void pushGray1(uint8_t value, const TextRenderArgs &args) {
    if (value && hasFlag(args.flags, TextRenderFlags::USE_FORE_COLOR)) {
      push444(args.foreColor);
    } else if (hasFlag(args.flags, TextRenderFlags::USE_BACK_COLOR)) {
      push444(args.backColor);
    } else {
      skip();
    }
  }
  XMC_INLINE void push444(uint16_t color) {
    if ((x++ & 1) == 0) {
      *(ptr++) = (color >> 4) & 0xFF;
      *ptr = (*ptr & 0x0F) | ((color & 0x0F) << 4);
    } else {
      *ptr = (*ptr & 0xF0) | ((color >> 8) & 0x0F);
      ptr++;
      *(ptr++) = color & 0xFF;
    }
  }
  XMC_INLINE void push565(uint16_t color) { push444(convert565To444(color)); }
  XMC_INLINE void push4444(uint16_t color) {
    uint16_t a1 = color & 0xF000;
    if (a1 == 0xF000) {
      push444(color);
    } else if (a1 == 0) {
      skip();
    } else {
      push444(blend4444To444(peek(), color));
    }
  }
  XMC_INLINE void skip() { ptr += 1 + (x++ & 1); }
  XMC_INLINE void flush() {}
};

class RasterScan565 {
 public:
  uint16_t *ptr;
  XMC_INLINE RasterScan565(uint16_t *data, int x) : ptr(data + x) {}
  XMC_INLINE ~RasterScan565() { flush(); }
  XMC_INLINE uint16_t peek() const { return *ptr; }
  XMC_INLINE uint16_t pop() { return *ptr++; }
  XMC_INLINE void pushGray1(uint8_t value, const TextRenderArgs &args) {
    if (value && hasFlag(args.flags, TextRenderFlags::USE_FORE_COLOR)) {
      *ptr++ = args.foreColor;
    } else if (hasFlag(args.flags, TextRenderFlags::USE_BACK_COLOR)) {
      *ptr++ = args.backColor;
    } else {
      ptr++;
    }
  }
  XMC_INLINE void push444(uint16_t color) { push565(convert444To565(color)); }
  XMC_INLINE void push565(uint16_t color) { *ptr++ = color; }
  XMC_INLINE void push4444(uint16_t color) {
    uint16_t a1 = color & 0xF000;
    if (a1 == 0xF000) {
      push444(color);
    } else if (a1 == 0) {
      skip();
    } else {
      push444(blend4444To565(peek(), color));
    }
  }
  XMC_INLINE void skip(int n = 1) { ptr += n; }
  XMC_INLINE void flush() {}
};

class RasterScan4444 {
 public:
  uint16_t *ptr;
  XMC_INLINE RasterScan4444(uint16_t *data, int x) : ptr(data + x) {}
  XMC_INLINE ~RasterScan4444() { flush(); }
  XMC_INLINE uint16_t peek() const { return *ptr; }
  XMC_INLINE uint16_t pop() { return *ptr++; }
  XMC_INLINE void pushGray1(uint8_t value, const TextRenderArgs &args) {
    if (value && hasFlag(args.flags, TextRenderFlags::USE_FORE_COLOR)) {
      push4444(args.foreColor);
    } else if (hasFlag(args.flags, TextRenderFlags::USE_BACK_COLOR)) {
      push4444(args.backColor);
    } else {
      ptr++;
    }
  }
  XMC_INLINE void push444(uint16_t color) { *(ptr++) = color | 0xF000; }
  XMC_INLINE void push565(uint16_t color) {
    *(ptr++) = convert565To444(color) | 0xF000;
  }
  XMC_INLINE void push4444(uint16_t color) {
    uint32_t a1 = color & 0xF000;
    if (a1 == 0xF000) {
      *(ptr++) = color;
    } else if (a1 == 0) {
      skip();
    } else {
      uint16_t orig = peek();
      uint32_t a2 = orig & 0xF000;
      *(ptr++) = blend4444To444(orig, color) | ((a1 * a2 / 0xF000) & 0xF000);
    }
  }
  XMC_INLINE void skip(int n = 1) { ptr += n; }
  XMC_INLINE void flush() {}
};

}  // namespace xmc

#endif
