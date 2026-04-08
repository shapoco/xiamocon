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
    offsetBits = glyph.bitmapOffset * 8;
    srcShiftReg = 0;
    ySrc = 0;
  }
  return metrics;
}

bool GlyphRendererAgfx::onRenderNextLine(uint8_t *buff) {
  if (ySrc >= glyph.height) return false;

  uint8_t outShiftReg = 0;
  memset(buff, 0, (glyph.width * xScale + 7) / 8 * sizeof(uint8_t));
  int dx = 0;
  for (int sx = 0; sx < glyph.width; sx++) {
    if (offsetBits % 8 == 0) {
      srcShiftReg = font->bitmap[offsetBits / 8];
    }
    for (int s = 0; s < xScale; s++, dx++) {
      if (srcShiftReg & 0x80) {
        outShiftReg |= (0x80 >> (dx % 8));
      }
      if (dx % 8 == 7) {
        if (dx < MAX_GLYPH_WIDTH) {
          buff[dx / 8] = outShiftReg;
        }
        outShiftReg = 0;
      }
    }
    offsetBits++;
    srcShiftReg <<= 1;
  }
  if (dx % 8 != 0) {
    if (dx < MAX_GLYPH_WIDTH) {
      buff[dx / 8] = outShiftReg;
    }
  }
  ySrc++;
  return true;
}

}  // namespace xmc
