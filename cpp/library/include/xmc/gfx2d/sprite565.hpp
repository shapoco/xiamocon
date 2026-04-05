/**
 * @file sprite565.hpp
 * @brief Sprite class for 16-bit RGB565 images
 */

#ifndef XMC_SPRITE565_HPP
#define XMC_SPRITE565_HPP

#include "xmc/gfx2d/sprite.hpp"

namespace xmc {

/** Sprite class for 16-bit RGB565 images */
class Sprite565Class : public SpriteClass {
 public:
  Sprite565Class(int width, int height, uint32_t stride, void *data,
                 bool autoFree = false)
      : SpriteClass(PixelFormat::RGB565, width, height, stride, data,
                    autoFree) {}

  Sprite565Class(int width, int height, XmcRamCap caps = XMC_RAM_CAP_DMA)
      : SpriteClass(PixelFormat::RGB565, width, height,
                    sizeof(uint16_t) * width,
                    xmcMalloc(sizeof(uint16_t) * width * height, caps), true) {
  }

 protected:
  void onSetPixel(int x, int y, uint16_t color) override;
  uint16_t onGetPixel(int x, int y) const override;
  void onFillRect(int x, int y, int w, int h, uint16_t color) override;
  void onFillSmokeRect(int x, int y, int w, int h, bool light) override;
  void onDrawImage(const Sprite &image, int dx, int dy, int w, int h,
                     int sx, int sy) override;
  XmcStatus onStartTransferToDisplay(int dx, int dy, int sy,
                                            int h) override;
};

static inline Sprite createSprite565(int width, int height,
                                     XmcRamCap caps = XMC_RAM_CAP_DMA) {
  return std::make_shared<Sprite565Class>(width, height, caps);
}

static inline Sprite createSprite565(int width, int height, uint32_t stride,
                                     void *data, bool autoFree = false) {
  return std::make_shared<Sprite565Class>(width, height, stride, data,
                                          autoFree);
}

}  // namespace xmc

#endif
