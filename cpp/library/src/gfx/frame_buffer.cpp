#include "xmc/gfx2d/frame_buffer.hpp"
#include "xmc/display.hpp"

namespace xmc {

FrameBuffer::FrameBuffer(PixelFormat fmt, bool doubleBuffered,
                                   int width, int height)
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
  if (numBuffers >= 2 && transferInProgress) {
    XMC_ERR_RET(completeTransferToDisplay());
    transferInProgress = false;
  }
  XMC_ERR_RET(startTransferToDisplay(getBackBuffer(), 0, 0));
  transferInProgress = true;
  frontIndex = (frontIndex + 1) % numBuffers;
  return XMC_OK;
}

}  // namespace xmc