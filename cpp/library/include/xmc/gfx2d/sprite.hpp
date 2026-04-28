#ifndef XMC_SPRITE_HPP
#define XMC_SPRITE_HPP

#include "gfxfont.h"
#include "xmc/battery.hpp"
#include "xmc/display.hpp"
#include "xmc/geo.hpp"
#include "xmc/gfx2d/gfx2d_common.hpp"
#include "xmc/hw/heap.hpp"

#include <memory>
#include <string>

#include "xmc/font/ShapoSansP_s08c07.h"

namespace xmc {

static constexpr int GFX2D_STRIDE_AUTO = 0;

class SpriteClass;
using Sprite = std::shared_ptr<SpriteClass>;

// todo: move to somewhere else
struct GraphicsState2D {
  const GFXfont *font = nullptr;
  int fontSize = 1;
  int fontOffsetY = 0;
  int fontHeight = 0;
  DevColor textColor = 0;
  DevColor backColor = 0;
  TextRenderFlags textFlags = TextRenderFlags::DRAW_FORE;
  int cursorX = 0;
  int cursorY = 0;
  Rect clipRect;
};

/** Calculate the stride (in bytes) for a given pixel format and width */
static constexpr inline uint32_t calcStride(PixelFormat format, int width) {
  switch (format) {
    default:
    case PixelFormat::RGB565: return width * 2;
    case PixelFormat::ARGB4444: return width * 2;
    case PixelFormat::RGB444: return (width * 3 + 1) / 2;
    case PixelFormat::GRAY1: return (width + 7) / 8;
  }
}

static inline Sprite createSprite565(int width, int height,
                                     XmcHeapCap caps = XMC_HEAP_CAP_DMA) {
  return std::make_shared<SpriteClass>(PixelFormat::RGB565, width, height,
                                       caps);
}

static inline Sprite createSprite565(int width, int height, void *data,
                                     uint32_t stride = GFX2D_STRIDE_AUTO,
                                     bool autoFree = false) {
  return std::make_shared<SpriteClass>(PixelFormat::RGB565, width, height,
                                       stride, data, autoFree);
}

static inline Sprite createSprite444(int width, int height,
                                     XmcHeapCap caps = XMC_HEAP_CAP_DMA) {
  return std::make_shared<SpriteClass>(PixelFormat::RGB444, width, height,
                                       caps);
}

static inline Sprite createSprite444(int width, int height, void *data,
                                     uint32_t stride = GFX2D_STRIDE_AUTO,
                                     bool autoFree = false) {
  return std::make_shared<SpriteClass>(PixelFormat::RGB444, width, height,
                                       stride, data, autoFree);
}

static inline Sprite createSprite4444(int width, int height,
                                      XmcHeapCap caps = XMC_HEAP_CAP_DMA) {
  return std::make_shared<SpriteClass>(PixelFormat::ARGB4444, width, height,
                                       caps);
}

static inline Sprite createSprite4444(int width, int height, void *data,
                                      uint32_t stride = GFX2D_STRIDE_AUTO,
                                      bool autoFree = false) {
  return std::make_shared<SpriteClass>(PixelFormat::ARGB4444, width, height,
                                       stride, data, autoFree);
}

static inline Sprite createSpriteGray1(int width, int height,
                                       XmcHeapCap caps = XMC_HEAP_CAP_DMA) {
  return std::make_shared<SpriteClass>(PixelFormat::GRAY1, width, height, caps);
}

static inline Sprite createSpriteGray1(int width, int height, void *data,
                                       uint32_t stride = GFX2D_STRIDE_AUTO,
                                       bool autoFree = false) {
  return std::make_shared<SpriteClass>(PixelFormat::GRAY1, width, height,
                                       stride, data, autoFree);
}

/**
 * Base class for sprites. This is a template class that takes a pixel format
 * and a pixel type as template parameters. The pixel format is used to
 * determine how the pixel data is stored. The pixel type is used to represent
 * the color of a pixel in the sprite. The Sprite class provides basic
 * functionality for managing a sprite, such as setting and getting pixel
 * values, filling rectangles, and starting a transfer to the display. The
 * actual implementation of these functions is left to derived classes that
 * specialize the template for specific pixel formats and types.
 */
class SpriteClass {
 public:
  const PixelFormat format;
  const int width;
  const int height;
  const uint32_t stride;
  void *data;
  const bool autoFree;
  GraphicsState2D textState;

  SpriteClass(PixelFormat format, int width, int height, uint32_t stride,
              void *data, bool autoFree)
      : format(format),
        width(width),
        height(height),
        stride(stride <= 0 ? calcStride(format, width) : stride),
        data(data),
        autoFree(autoFree) {}

  SpriteClass(PixelFormat format, int width, int height,
              XmcHeapCap caps = XMC_HEAP_CAP_DMA)
      : format(format),
        width(width),
        height(height),
        stride(calcStride(format, width)),
        data(xmcMalloc(calcStride(format, width) * height, caps)),
        autoFree(true) {}

  ~SpriteClass() {
    if (autoFree && data) {
      xmcFree(data);
      data = nullptr;
    }
  }

  inline void *linePtr(int y) const {
    if (y < 0 || y >= height) return nullptr;
    return (uint8_t *)data + stride * y;
  }
};

// todo: move to somewhere else
XmcStatus startTransferToDisplay(Sprite sprite, int dx, int dy);
XmcStatus completeTransferToDisplay();

}  // namespace xmc

#endif