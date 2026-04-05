/**
 * @file sprite444.hpp
 * @brief Sprite class for 12-bit RGB444 images
 */

#ifndef XMC_SPRITE444_HPP
#define XMC_SPRITE444_HPP

#include "xmc/gfx2d/sprite.hpp"

namespace xmc {

/** Calculate the stride (in bytes) for a 12-bit RGB444 image */
static inline uint32_t stride444(int width) { return (width * 12 + 7) / 8; }

/** Sprite class for 12-bit RGB444 images */
class Sprite444Class : public SpriteClass {
 public:
  Sprite444Class(int width, int height, uint32_t stride, void *data,
                 bool autoFree = false)
      : SpriteClass(PixelFormat::RGB444, width, height, stride444(width), data,
                    autoFree) {}

  Sprite444Class(int width, int height, XmcRamCap caps = XMC_RAM_CAP_DMA)
      : SpriteClass(PixelFormat::RGB444, width, height, stride444(width),
                    xmcMalloc(stride444(width) * height, caps), true) {}

 protected:
  void onSetPixel(int x, int y, uint16_t color) override;
  uint16_t onGetPixel(int x, int y) const override;
  void onFillRect(int x, int y, int w, int h, uint16_t color) override;
  void onFillSmokeRect(int x, int y, int w, int h, bool light) override;
  void onDrawImage(const Sprite &image, int dx, int dy, int w, int h, int sx,
                   int sy) override;
  XmcStatus onStartTransferToDisplay(int dx, int dy, int sy, int h) override;
};

static inline Sprite createSprite444(int width, int height,
                                     XmcRamCap caps = XMC_RAM_CAP_DMA) {
  return std::make_shared<Sprite444Class>(width, height, caps);
}

static inline Sprite createSprite444(int width, int height, uint32_t stride,
                                     void *data, bool autoFree = false) {
  return std::make_shared<Sprite444Class>(width, height, stride, data,
                                          autoFree);
}

struct Scanner444 {
  uint8_t *ptr;
  int x;

  XMC_INLINE Scanner444(SpriteClass &sprite, int x, int y)
      : ptr((uint8_t *)sprite.linePtr(y) + x * 3 / 2), x(x) {}

  XMC_INLINE Scanner444(Sprite &sprite, int x, int y)
      : Scanner444(*sprite, x, y) {}

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

  XMC_INLINE void push4444(uint16_t color) {
    uint16_t a1 = color & 0xF000;
    if (a1 == 0xF000) {
      push444(color);
    } else if (a1 == 0) {
      skip();
    } else {
      uint16_t a2 = 16 - a1;
      uint16_t c = peek();
      uint32_t tmp1 =
          ((color & 0xF00) << 12) | ((color & 0x0F0) << 6) | (color & 0x00F);
      uint32_t tmp2 = ((c & 0xF00) << 12) | ((c & 0x0F0) << 6) | (c & 0x00F);
      tmp1 = tmp1 * a1 + tmp2 * a2;
      c = ((tmp1 >> 16) & 0xF00) | ((tmp1 >> 10) & 0x0F0) |
          ((tmp1 >> 4) & 0x00F);
      push444(c);
    }
  }

  XMC_INLINE uint16_t peek() const {
    if ((x & 1) == 0) {
      return ((uint16_t)(*ptr) << 4) | ((*ptr >> 4) & 0x0F);
    } else {
      return ((uint16_t)(*ptr & 0x0F) << 8) | (*(ptr + 1));
    }
  }

  XMC_INLINE void skip() { ptr += 1 + (x++ & 1); }
};

}  // namespace xmc

#endif
