#include "xmc/gfx2d/graphics2d.hpp"
#include "xmc/display.hpp"

namespace xmc {

uint8_t lineBuffer[display::WIDTH * sizeof(RawColor)];

static void fillRectUnchecked(PixelFormat fmt, void *ptr, int x, int w, int h,
                              uint32_t stride, RawColor color);

void Graphics2DClass::setTarget(Sprite &s) {
  target = s;
  clearClipRect();
}

void Graphics2DClass::setClipRect(const Rect &rect) {
  state.clipRect = rect.intersect(getBounds());
}

void Graphics2DClass::clearClipRect() { state.clipRect = getBounds(); }

Rect Graphics2DClass::getBounds() {
  if (target) {
    return {0, 0, target->width, target->height};
  } else {
    return {0, 0, display::WIDTH, display::HEIGHT};
  }
}

void Graphics2DClass::fillRect(int x, int y, int w, int h, RawColor color) {
  Rect dstRect = {x, y, w, h};
  dstRect = dstRect.intersect(state.clipRect);

  if (dstRect.width <= 0 || dstRect.height <= 0) return;

  if (target) {
    fillRectUnchecked(target->format, target->linePtr(dstRect.y), dstRect.x,
                      dstRect.width, dstRect.height, target->stride, color);
  } else {
    auto fmt = display::getPixelFormat();
    fillRectUnchecked(fmt, lineBuffer, 0, dstRect.width, 1, 0, color);
    uint32_t writeSize = 0;
    switch (fmt) {
      case PixelFormat::RGB444: {
        writeSize = dstRect.width * 3 / 2;
      } break;
      case PixelFormat::RGB565: {
        writeSize = dstRect.width * 2;
      } break;
        // todo: support RGB666
    }

    for (int j = 0; j < dstRect.height; j++) {
      display::setWindow(dstRect.x, dstRect.y + j, dstRect.width, 1);
      display::writePixelsStart(lineBuffer, writeSize, false);
      display::writePixelsComplete();
    }
  }
}

void Graphics2DClass::drawRect(int x, int y, int w, int h, RawColor color) {
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

void Graphics2DClass::fillSmokeRect(int x, int y, int w, int h, bool white) {
  Rect dstRect = {x, y, w, h};
  dstRect = dstRect.intersect(state.clipRect);
  if (dstRect.width <= 0 || dstRect.height <= 0) return;
  if (target) {
    for (int j = 0; j < dstRect.height; j++) {
      switch (target->format) {
        case PixelFormat::RGB444: {
          RasterScan444 scan((uint8_t *)target->linePtr(dstRect.y + j),
                             dstRect.x);
          uint16_t add = white ? 0x888 : 0;
          for (int i = 0; i < dstRect.width; i++) {
            scan.push444(((scan.peek() & 0xEEE) >> 1) | add);
          }
        } break;
        case PixelFormat::RGB565: {
          uint16_t *ptr =
              (uint16_t *)target->linePtr(dstRect.y + j) + dstRect.x;
          uint16_t add = white ? 0x8410 : 0;
          for (int i = 0; i < dstRect.width; i++) {
            uint16_t tmp = ((ptr[i] << 8) & 0xFF00) | ((ptr[i] >> 8) & 0x00FF);
            tmp = ((tmp & 0xF7DE) >> 1) | add;
            ptr[i] = ((tmp << 8) & 0xFF00) | ((tmp >> 8) & 0x00FF);
          }
        } break;
        case PixelFormat::ARGB4444: {
          uint16_t *ptr =
              (uint16_t *)target->linePtr(dstRect.y + j) + dstRect.x;
          uint16_t add = white ? 0x0888 : 0;
          for (int i = 0; i < dstRect.width; i++) {
            ptr[i] = (ptr[i] & 0xF000) | ((ptr[i] & 0x0EEE) >> 1) | add;
          }
        } break;
      }
    }
  } else {
    fillRect(x, y, w, h, white ? 0xFFFFFFFF : 0);
  }
}

void Graphics2DClass::drawImage(const Sprite &image, int dx, int dy, int w,
                                int h, int sx, int sy,
                                const TextRenderArgs &tra) {
  Rect srcRect = {sx, sy, w, h};
  Rect dstRect = {dx, dy, w, h};
  Rect tmpClip = dstRect.intersect(state.clipRect);
  if (tmpClip.width <= 0 || tmpClip.height <= 0) return;
  int offsetX = tmpClip.x - dstRect.x;
  int offsetY = tmpClip.y - dstRect.y;
  srcRect.x += offsetX;
  srcRect.y += offsetY;
  srcRect.width = tmpClip.width;
  srcRect.height = tmpClip.height;
  dstRect = tmpClip;

  if (target) {
    for (int j = 0; j < dstRect.height; j++) {
      copyPixelString(target->format, target->linePtr(dstRect.y + j), dstRect.x,
                      image->format, image->linePtr(srcRect.y + j), srcRect.x,
                      srcRect.width, tra);
    }
  } else {
    PixelFormat dstFmt;
    switch (display::getPixelFormat()) {
      case PixelFormat::RGB444: dstFmt = PixelFormat::RGB444; break;
      case PixelFormat::RGB565: dstFmt = PixelFormat::RGB565; break;
      default:
        // todo: support RGB666
        return;
    }
    for (int j = 0; j < dstRect.height; j++) {
      copyPixelString(dstFmt, lineBuffer, 0, image->format,
                      image->linePtr(srcRect.y + j), srcRect.x, srcRect.width,
                      tra);
      display::setWindow(dstRect.x, dstRect.y + j, dstRect.width, 1);
      display::writePixelsStart(lineBuffer, dstRect.width * 2, false);
      display::writePixelsComplete();
    }
  }
}

void Graphics2DClass::setFont(const GFXfont *font, int size) {
  state.font = font;
  state.fontSize = size;
}

void Graphics2DClass::setCursor(int x, int y) {
  state.cursorX = x;
  state.cursorY = y;
}

void Graphics2DClass::setTextColor(RawColor fg) {
  state.textColor = fg;
  state.textFlags = TextRenderFlags::DRAW_FORE;
}

void Graphics2DClass::setTextColor(RawColor fg, RawColor bg) {
  state.textColor = fg;
  state.backColor = bg;
  state.textFlags = TextRenderFlags::DRAW_FORE | TextRenderFlags::DRAW_BACK;
}

void Graphics2DClass::drawString(const char *str) {
  GlyphRendererAgfx renderer(state.font, state.fontSize, state.fontSize);
  if (!state.font || !str) return;
  int x = state.cursorX;
  for (const char *p = str; *p; p++) {
    if (*p == '\n') {
      x = state.cursorX;
      state.cursorY += state.font->yAdvance * state.fontSize;
    } else {
      x += drawChar(renderer, x, state.cursorY, *p);
    }
  }
  state.cursorX = x;
}

int Graphics2DClass::drawChar(GlyphRenderer &renderer, int x, int y, int code) {
  if (!state.font) return 0;
  if (code < state.font->first || code > state.font->last) return 0;

  GlyphMetrics metrics = renderer.beginRender(code);

  int dx = x + metrics.xOffset;
  int dy = y + metrics.yOffset;
  int sx = 0;
  int w = metrics.width;
  if (dx < 0) {
    sx = -dx;
    w += dx;
    dx = 0;
  }

  if (dx + w >= state.clipRect.right()) {
    w = state.clipRect.right() - dx;
  }

  PixelFormat dstFmt;
  uint8_t *dstPtr = nullptr;
  int dstStride = 0;
  if (target) {
    dstFmt = target->format;
    dstPtr = (uint8_t *)target->linePtr(dy >= 0 ? dy : 0);
    dstStride = target->stride;
  } else {
    switch (display::getPixelFormat()) {
      case PixelFormat::RGB444: dstFmt = PixelFormat::RGB444; break;
      case PixelFormat::RGB565: dstFmt = PixelFormat::RGB565; break;
      default:
        // todo: support RGB666
        return 0;
    }
    dstPtr = lineBuffer;
    dstStride = 0;
  }

  TextRenderArgs tra;
  tra.foreColor = state.textColor;
  tra.backColor = state.backColor;
  tra.flags = state.textFlags;
  bool hasFore = hasFlag(tra.flags, TextRenderFlags::DRAW_FORE);
  bool hasBack = hasFlag(tra.flags, TextRenderFlags::DRAW_BACK);
  if (!target) {
    if (!hasFore && !hasBack) {
      tra.foreColor = 0xFFFFFFFF;
      tra.backColor = 0x00000000;
    } else if (hasFore) {
      tra.backColor = tra.foreColor ? 0x00000000 : 0xFFFFFFFF;
    } else if (hasBack) {
      tra.foreColor = tra.backColor ? 0x00000000 : 0xFFFFFFFF;
    }
    tra.flags = TextRenderFlags::DRAW_FORE | TextRenderFlags::DRAW_BACK;
  }

  for (int j = 0; j < metrics.height; j++, dy++) {
    if (dy >= state.clipRect.bottom()) break;
    bool skip = (dy < 0);

    int rdx = target ? dx : 0;
    renderer.renderNextLine(dstFmt, dstPtr, rdx, w, sx, tra, skip);
    dstPtr += dstStride;

    if (!skip && !target) {
      display::setWindow(dx, dy, w, 1);
      display::writePixelsStart(lineBuffer, w * 2);
      display::writePixelsComplete();
    }
  }
  return metrics.xAdvance;
}

static void fillRectUnchecked(PixelFormat fmt, void *ptr, int x, int w, int h,
                              uint32_t stride, RawColor color) {
  switch (fmt) {
    case PixelFormat::GRAY1: {
      TextRenderArgs tra;
      tra.foreColor = color;
      tra.backColor = 0;
      tra.flags = TextRenderFlags::DRAW_FORE;
      uint8_t c = color ? 1 : 0;
      for (int j = 0; j < h; j++) {
        RasterScanGray1 scan((uint8_t *)(ptr + j * stride), x);
        for (int i = 0; i < w; i++) {
          scan.pushGray1(1, tra);
        }
      }
    } break;

    case PixelFormat::RGB444: {
      uint16_t c = color;
      for (int j = 0; j < h; j++) {
        RasterScan444 scan((uint8_t *)(ptr + j * stride), x);
        for (int i = 0; i < w; i++) {
          scan.push444(c);
        }
      }
    } break;

    case PixelFormat::RGB565: {
      uint16_t c = color;
      for (int j = 0; j < h; j++) {
        RasterScan565 scan((uint16_t *)(ptr + j * stride), x);
        for (int i = 0; i < w; i++) {
          scan.push565(c);
        }
      }
    } break;

    case PixelFormat::ARGB4444: {
      uint16_t c = color;
      for (int j = 0; j < h; j++) {
        RasterScan4444 scan((uint16_t *)(ptr + j * stride), x);
        for (int i = 0; i < w; i++) {
          scan.push4444(c);
        }
      }
    } break;

    default: break;
  }
}

}  // namespace xmc
