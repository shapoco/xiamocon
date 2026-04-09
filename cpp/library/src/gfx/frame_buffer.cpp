#include "xmc/gfx2d/frame_buffer.hpp"
#include "xmc/display.hpp"
#include "xmc/hw/timer.hpp"

namespace xmc {

FrameBuffer::FrameBuffer(PixelFormat fmt, bool doubleBuffered, int width,
                         int height)
    : numBuffers(doubleBuffered ? 2 : 1) {
  {
    for (int i = 0; i < numBuffers; i++) {
      if (fmt == PixelFormat::RGB565) {
        sprites[i] = createSprite565(width, height);
      } else if (fmt == PixelFormat::RGB444) {
        sprites[i] = createSprite444(width, height);
      }
    }
  }
}

XmcStatus FrameBuffer::beginRender() {
  if (numBuffers < 2 && transferInProgress) {
    XMC_ERR_RET(completeTransferToDisplay());
    transferInProgress = false;
  }
  return XMC_OK;
}

XmcStatus FrameBuffer::endRender(bool transferToDisplay) {
  if (!transferToDisplay) return XMC_OK;

  uint64_t nowUs = getTimeUs();
  if (nowUs >= fpsLastUpdateUs + 1000000) {
    float fps = fpsFrameCount * 1000000.0f / ((nowUs - fpsLastUpdateUs));
    snprintf(fpsString, sizeof(fpsString), "%5.2f FPS", fps);
    fpsFrameCount = 0;
    fpsLastUpdateUs = nowUs;
  }

  if (hasFlag(flags, FrameBufferFlags::SHOW_STATUS_BAR)) {
    Graphics2D gfx = createGraphics();
    renderStatusBar(gfx);
  }
  if (hasFlag(flags, FrameBufferFlags::SHOW_DEBUG_INFO)) {
    Graphics2D gfx = createGraphics();
    renderDebugBar(gfx);
  }

  if (numBuffers >= 2 && transferInProgress) {
    XMC_ERR_RET(completeTransferToDisplay());
    transferInProgress = false;
  }
  XMC_ERR_RET(startTransferToDisplay(getBackBuffer(), 0, 0));
  transferInProgress = true;
  frontIndex = (frontIndex + 1) % numBuffers;
  return XMC_OK;
}

XmcStatus FrameBuffer::renderStatusBar(Graphics2D &gfx) {
  int w = gfx->getBounds().width;
  int h = STATUS_BAR_HEIGHT;
  int baseLine = 1;

  GraphicsState2D backup = gfx->getState();

  gfx->setFont(&ShapoSansP_s08c07, 1);
  gfx->setTextColor(0xFFFF);
  gfx->fillSmokeRect(0, 0, w, h, false);

  if (hasFlag(flags, FrameBufferFlags::SHOW_FPS)) {
    gfx->setCursor(1, baseLine);
    gfx->drawString(fpsString);
    fpsFrameCount++;
  }

  int batMv = battery::getVoltageMv();
  if (batMv != lastVoltageMv) {
    lastVoltageMv = batMv;
    snprintf(batString, sizeof(batString), "%4.2fV", batMv / 1000.0f);
  }
  if (hasFlag(flags, FrameBufferFlags::SHOW_BATTERY_LEVEL)) {
    constexpr int batMin = 3300;
    constexpr int batMax = 4200;
    int batteryGuageWidth = 12 * (lastVoltageMv - batMin) / (batMax - batMin);
    if (batteryGuageWidth < 0) {
      batteryGuageWidth = 0;
    } else if (batteryGuageWidth > 12) {
      batteryGuageWidth = 12;
    }
    gfx->drawRect(w - 50, 1, 15, h - 2, 0xFFFF);
    gfx->fillRect(w - 34, 4, 2, h - 7, 0xFFFF);
    gfx->fillRect(w - 48, 3, batteryGuageWidth, h - 5, 0xFFFF);
    gfx->setCursor(w - 30, baseLine);
    gfx->drawString(batString);
  }

  gfx->setState(backup);

  return XMC_OK;
}

XmcStatus FrameBuffer::renderDebugBar(Graphics2D &gfx) {
  XmcStatus err;
  const char *file;
  int line;
  xmcGetLastError(&err, &file, &line);
  if (err == XMC_OK) return XMC_OK;

  int w = gfx->getBounds().width;
  int h = 10;

  char buf[64];
  GraphicsState2D backup = gfx->getState();
  gfx->fillSmokeRect(0, gfx->getBounds().height - h, w, h, false);
  gfx->setFont(&ShapoSansP_s08c07, 1);
  snprintf(buf, sizeof(buf), "ERR 0x%X: L%d in %s", err, line, file);
  gfx->setCursor(0, gfx->getBounds().height - 2);
  gfx->setTextColor(0xFFFF);
  gfx->drawString(buf);
  gfx->setState(backup);
  return XMC_OK;
}

}  // namespace xmc