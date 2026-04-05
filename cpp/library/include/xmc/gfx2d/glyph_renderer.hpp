#ifndef XMC_GFX_FONT_DECODER_HPP
#define XMC_GFX_FONT_DECODER_HPP

#include "gfxfont.h"
#include "xmc/gfx2d/gfx_common.hpp"

#include <string.h>

namespace xmc {

class GlyphRenderer {
 public:
  static constexpr int MAX_GLYPH_WIDTH = 128;

  const int xScale, yScale;
  GlyphRenderer(int xScale = 1, int yScale = 1)
      : xScale(xScale), yScale(yScale) {}

  virtual int getYAdvance() const = 0;

  GlyphMetrics beginRender(int code);

  bool renderNextLine(PixelFormat fmt, void *dst, int dstX, int dstW, int srcX,
                      const TextRenderArgs &tra, bool skip = false);

 private:
  uint8_t binaryBuff[MAX_GLYPH_WIDTH / 8];
  GlyphMetrics metrics;
  int yCoarse = 0;
  int yFine = 0;

 protected:
  int code = 0;

  virtual GlyphMetrics onBeginDecode(int code) = 0;
  virtual bool onRenderNextLine(uint8_t *buff) = 0;
};

}  // namespace xmc
#endif
