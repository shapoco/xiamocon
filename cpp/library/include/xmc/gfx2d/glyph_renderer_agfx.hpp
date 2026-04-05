#ifndef XMC_GFX_GLYPH_DECODER_AGFX_HPP
#define XMC_GFX_GLYPH_DECODER_AGFX_HPP

#include "xmc/gfx2d/glyph_renderer.hpp"

namespace xmc {

class GlyphRendererAgfx : public GlyphRenderer {
 public:
  GlyphRendererAgfx(const GFXfont *font, int xScale = 1, int yScale = 1)
      : GlyphRenderer(xScale, yScale), font(font) {}

  int getYAdvance() const override { return font->yAdvance * yScale; }

 protected:
  GFXglyph glyph;
  int yDecode = 0;
  uint32_t offsetBits = 0;
  uint8_t srcShiftReg = 0;

  GlyphMetrics onBeginDecode(int code) override;
  bool onRenderNextLine(uint8_t *buff) override;

 private:
  const GFXfont *font;
};

}  // namespace xmc

#endif
