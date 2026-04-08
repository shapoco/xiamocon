#include "xmc/gfx2d/glyph_renderer.hpp"
#include "xmc/gfx2d/raster_scan.hpp"

namespace xmc {

GlyphMetrics GlyphRenderer::beginRender(int code) {
  this->code = code;
  metrics = onBeginDecode(code);
  GlyphMetrics scaledMetrics = metrics;
  scaledMetrics.width *= xScale;
  scaledMetrics.height *= yScale;
  scaledMetrics.xOffset *= xScale;
  scaledMetrics.yOffset *= yScale;
  scaledMetrics.xAdvance *= xScale;
  yCoarse = 0;
  yFine = 0;
  return scaledMetrics;
}

bool GlyphRenderer::renderNextLine(PixelFormat fmt, void *dst, int dstX,
                                   int dstW, int srcX,
                                   const TextRenderArgs &tra, bool skip) {
  if (yFine == 0) {
    if (!onRenderNextLine(binaryBuff)) return false;
  }

  if (!skip) {
    copyPixelString(fmt, dst, dstX, PixelFormat::GRAY1, binaryBuff, srcX,
                    metrics.width * xScale, tra);
  }

  yFine++;
  if (yFine >= yScale) {
    yFine = 0;
    yCoarse++;
  }

  return true;
}

}  // namespace xmc
