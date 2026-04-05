#ifndef XMC_SPRITE_HPP
#define XMC_SPRITE_HPP

#include "gfxfont.h"
#include "xmc/battery.hpp"
#include "xmc/display.hpp"
#include "xmc/geo.hpp"
#include "xmc/gfx2d/gfx_common.hpp"
#include "xmc/hw/ram.hpp"

#include <memory>
#include <string>

#include "xmc/font/ShapoSansP_s08c07.h"

namespace xmc {

class SpriteClass;
using Sprite = std::shared_ptr<SpriteClass>;

// todo: move to graphics2d.hpp
struct GraphicsState2D {
  const GFXfont *font = nullptr;
  int fontSize = 1;
  uint16_t textColor = 0;
  int cursorX = 0;
  int cursorY = 0;
  Rect clipRect;
};

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
        stride(stride),
        data(data),
        autoFree(autoFree) {}

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

  __attribute__((deprecated))
  void setPixel(int x, int y, RawColor color) {
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    onSetPixel(x, y, color);
  }

  __attribute__((deprecated))
  RawColor getPixel(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) return 0;
    return onGetPixel(x, y);
  }

  __attribute__((deprecated))
  void setFont(const GFXfont *font, int size = 1) {
    textState.font = font;
    textState.fontSize = size;
  }

  __attribute__((deprecated))
  void setCursor(int x, int y) {
    textState.cursorX = x;
    textState.cursorY = y;
  }

  __attribute__((deprecated))
  void setTextColor(RawColor color) { textState.textColor = color; }

  __attribute__((deprecated))
  void drawString(const char *str) {
    if (!textState.font || !str) return;
    int x = textState.cursorX;
    for (const char *p = str; *p; p++) {
      if (*p == '\n') {
        x = textState.cursorX;
        textState.cursorY += textState.font->yAdvance * textState.fontSize;
      } else {
        x += onDrawChar(x, textState.cursorY, *p);
      }
    }
    textState.cursorX = x;
  }

  __attribute__((deprecated))
  XmcStatus startTransferToDisplay(int dx, int dy) {
    return onStartTransferToDisplay(dx, dy, 0, height);
  }

  __attribute__((deprecated))
  XmcStatus completeTransfer() { return display::writePixelsComplete(); }

  __attribute__((deprecated))
  void clear(RawColor color) { onFillRect(0, 0, width, height, color); }

  __attribute__((deprecated))
  void fillRect(int x, int y, int w, int h, RawColor color) {
    if (w < 0) {
      x += w;
      w = -w;
    }
    if (h < 0) {
      y += h;
      h = -h;
    }
    clipRect(&x, &y, &w, &h, width, height);
    if (w <= 0 || h <= 0) return;
    onFillRect(x, y, w, h, color);
  }

  __attribute__((deprecated))
  void drawRect(int x, int y, int w, int h, RawColor color) {
    if (w < 0) {
      x += w;
      w = -w;
    }
    if (h < 0) {
      y += h;
      h = -h;
    }
    fillRect(x, y, w + 1, 1, color);
    fillRect(x, y + h, w + 1, 1, color);
    fillRect(x, y + 1, 1, h - 1, color);
    fillRect(x + w, y + 1, 1, h - 1, color);
  }

  __attribute__((deprecated))
  void fillSmokeRect(int x, int y, int w, int h, bool light = false) {
    if (w < 0) {
      x += w;
      w = -w;
    }
    if (h < 0) {
      y += h;
      h = -h;
    }
    clipRect(&x, &y, &w, &h, width, height);
    if (w <= 0 || h <= 0) return;
    onFillSmokeRect(x, y, w, h, light);
  }

  __attribute__((deprecated))
  inline void drawImage(const Sprite &image, int dx, int dy, int w, int h,
                        int sx, int sy) {
    Rect view = {0, 0, width, height};
    Rect dst = {dx, dy, w, h};
    dst = dst.intersect(view);
    if (dst.width <= 0 || dst.height <= 0) return;
    sx += dst.x - dx;
    sy += dst.y - dy;
    dx = dst.x;
    dy = dst.y;
    w = dst.width;
    h = dst.height;
    Rect img = {0, 0, image->width, image->height};
    Rect src = {sx, sy, w, h};
    src = src.intersect(img);
    if (src.width <= 0 || src.height <= 0) return;
    dx += src.x - sx;
    dy += src.y - sy;
    w = src.width;
    h = src.height;
    onDrawImage(image, dx, dy, w, h, sx, sy);
  }

 protected:
  virtual void onSetPixel(int x, int y, RawColor color) = 0;
  virtual RawColor onGetPixel(int x, int y) const = 0;
  virtual void onFillRect(int x, int y, int w, int h, RawColor color) = 0;
  virtual void onFillSmokeRect(int x, int y, int w, int h, bool light) = 0;
  virtual void onDrawImage(const Sprite &image, int dx, int dy, int w, int h,
                           int sx, int sy) = 0;
  virtual XmcStatus onStartTransferToDisplay(int dx, int dy, int sy, int h) = 0;
  virtual int onDrawChar(int x, int y, char c) {
    if (!textState.font) return 0;
    if (c < textState.font->first || c > textState.font->last) return 0;
    GFXglyph *glyph = &textState.font->glyph[c - textState.font->first];
    int x0 = x + glyph->xOffset * textState.fontSize;
    int y0 = y + glyph->yOffset * textState.fontSize;
    int w = glyph->width * textState.fontSize;
    int h = glyph->height * textState.fontSize;
    uint8_t *bitmap = textState.font->bitmap + glyph->bitmapOffset;
    uint8_t byte = 0;
    int ibit = 0;
    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        if (ibit == 0) {
          byte = *bitmap++;
        }
        if (byte & 0x80) {
          if (textState.fontSize == 1) {
            setPixel(x0 + i, y0 + j, textState.textColor);
          } else {
            fillRect(x0 + i * textState.fontSize, y0 + j * textState.fontSize,
                     textState.fontSize, textState.fontSize,
                     textState.textColor);
          }
        }
        byte <<= 1;
        ibit = (ibit + 1) % 8;
      }
    }
    return glyph->xAdvance * textState.fontSize;
  }
};

}  // namespace xmc

#endif