/**
 * @file sprite4444.hpp
 * @brief Sprite class for 16-bit ARGB4444 images
 */

#ifndef XMC_SPRITE4444_HPP
#define XMC_SPRITE4444_HPP

#include "xmc/gfx2d/sprite.hpp"

namespace xmc {

/** Sprite class for 16-bit ARGB4444 images */
class Sprite4444Class : public SpriteClass {
 public:
  Sprite4444Class(int width, int height, uint32_t stride, void *data,
                  bool autoFree = false)
      : SpriteClass(PixelFormat::ARGB4444, width, height,
                    (stride <= 0) ? (uint32_t)(width * sizeof(uint16_t)) : stride, data,
                    autoFree) {}

  Sprite4444Class(int width, int height, XmcRamCap caps = XMC_RAM_CAP_DMA)
      : SpriteClass(PixelFormat::ARGB4444, width, height,
                     (uint32_t)(width * sizeof(uint16_t)),
                    xmcMalloc(width * height * sizeof(uint16_t), caps), true) {}

 protected:
  void onSetPixel(int x, int y, uint16_t color) override;
  uint16_t onGetPixel(int x, int y) const override;
  void onFillRect(int x, int y, int w, int h, uint16_t color) override;
  void onFillSmokeRect(int x, int y, int w, int h, bool light) override;
  void onDrawImage(const Sprite &image, int dx, int dy, int w, int h, int sx,
                   int sy) override;
  XmcStatus onStartTransferToDisplay(int dx, int dy, int sy, int h) override;
};

static inline Sprite createSprite4444(int width, int height,
                                      XmcRamCap caps = XMC_RAM_CAP_DMA) {
  return std::make_shared<Sprite4444Class>(width, height, caps);
}

static inline Sprite createSprite4444(int width, int height, uint32_t stride,
                                      void *data, bool autoFree = false) {
  return std::make_shared<Sprite4444Class>(width, height, stride, data,
                                           autoFree);
}

}  // namespace xmc

#endif
