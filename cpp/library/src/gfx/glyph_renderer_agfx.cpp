#include "xmc/gfx2d/glyph_renderer_agfx.hpp"

namespace xmc {

GlyphMetrics GlyphRendererAgfx::onBeginDecode(int code) {
  GlyphMetrics metrics;
  if (code < font->first || code > font->last) {
    memset(&metrics, 0, sizeof(metrics));
  } else {
    glyph = font->glyph[code - font->first];
    metrics.width = glyph.width;
    metrics.height = glyph.height;
    metrics.xOffset = glyph.xOffset;
    metrics.yOffset = glyph.yOffset;
    metrics.xAdvance = glyph.xAdvance;
    yDecode = 0;
    offsetBits = glyph.bitmapOffset * 8;
    srcShiftReg = 0;
  }
  return metrics;
}

bool GlyphRendererAgfx::onRenderNextLine(uint8_t *buff) {
  if (yDecode >= glyph.height) return false;

  uint8_t outShiftReg = 0;
  memset(buff, 0, (glyph.width + 7) / 8 * sizeof(uint8_t));
  for (int x = 0; x < glyph.width; x++) {
    if (offsetBits % 8 == 0) {
      srcShiftReg = font->bitmap[offsetBits / 8];
    }
    for (int s = 0; s < xScale; s++) {
      if (srcShiftReg & 0x80) {
        outShiftReg |= (0x80 >> (x % 8));
      }
      if (x % 8 == 7) {
        if (x < MAX_GLYPH_WIDTH) {
          buff[x / 8] = outShiftReg;
        }
        outShiftReg = 0;
      }
    }
    offsetBits++;
    srcShiftReg <<= 1;
  }
  if (glyph.width % 8 != 0) {
    if (glyph.width < MAX_GLYPH_WIDTH) {
      buff[glyph.width / 8] = outShiftReg;
    }
  }
  yDecode++;
  return true;
}

}  // namespace xmc
