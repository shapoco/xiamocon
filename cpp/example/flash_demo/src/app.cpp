#include <xiamocon.hpp>

using namespace xmc;
using namespace xmc::input;

static constexpr PixelFormat DISPLAY_FORMAT = PixelFormat::RGB565;
static constexpr uint32_t TEST_SIZE = 64 * 1024;

FrameBuffer frameBuffer = createFrameBuffer(DISPLAY_FORMAT, false);
FpsKeeper fpsKeeper(30);

uint32_t prbsState = 0x12345678;
bool testDone = false;
XmcStatus testStatus = XMC_OK;
uint32_t testErrors = 0;
uint32_t testCrc32 = 0;

XmcStatus flashTest(uint32_t *numErrors, uint32_t *resultCrc);
uint32_t crc32(const uint8_t *data, size_t size);

AppConfig xmcAppGetConfig(void) {
  auto cfg = getDefaultAppConfig();
  cfg.displayPixelFormat = DISPLAY_FORMAT;
  cfg.speakerEnabled = false;
  return cfg;
}

void xmcAppSetup(void) {
  frameBuffer->enableFlag(FrameBufferFlags::SHOW_DEBUG_INFO);
}

void xmcAppLoop(void) {
  char buf[16];

  if (wasPressed(Button::A)) {
    testStatus = flashTest(&testErrors, &testCrc32);
    testDone = true;
  }

  fpsKeeper.waitVsync();
  if (!fpsKeeper.isFrameSkipping()) {
    int y = STATUS_BAR_HEIGHT + 10;
    frameBuffer->beginRender();

    auto gfx = frameBuffer->createGraphics();
    gfx->clear(gfx->devColor(Colors::BLACK));

    gfx->setTextColor(gfx->devColor(Colors::WHITE));

    gfx->setFont(&ShapoSansP_s21c16a01w03);
    gfx->setCursor(10, y);
    gfx->drawString("Flash Test");

    y += 25;
    gfx->setFont(&ShapoSansP_s12c09a01w02);
    gfx->setCursor(10, y);
    gfx->drawString("Press 'A' to start test");

    if (testDone) {
      y += 20;
      gfx->setCursor(10, y);
      gfx->drawString("Result: ");
      if (testStatus == XMC_OK) {
        gfx->setTextColor(gfx->devColor(Colors::GREEN));
        gfx->drawString("SUCCESS!");
      } else {
        gfx->setTextColor(gfx->devColor(Colors::RED));
        gfx->drawString("FAILED!");
      }
      gfx->setTextColor(gfx->devColor(Colors::WHITE));

      y += 20;
      gfx->setCursor(10, y);
      gfx->drawString("Error Code: ");
      snprintf(buf, sizeof(buf), "%u", (unsigned int)testStatus);
      gfx->drawString(buf);

      y += 20;
      gfx->setCursor(10, y);
      gfx->drawString("Number of Error Bytes: ");
      snprintf(buf, sizeof(buf), "%u", testErrors);
      gfx->drawString(buf);

      y += 20;
      gfx->setCursor(10, y);
      gfx->drawString("CRC32: ");
      snprintf(buf, sizeof(buf), "0x%08lX", testCrc32);
      gfx->drawString(buf);
    }

    frameBuffer->endRender();
  }
}

XmcStatus flashTest(uint32_t *numErrors, uint32_t *resultCrc) {
  XmcStatus sts;

  uint8_t *buff =
      static_cast<uint8_t *>(xmcMalloc(TEST_SIZE, XMC_HEAP_CAP_DMA));
  for (size_t i = 0; i < TEST_SIZE; i++) {
    buff[i] = prbsState & 0xFF;
    updateLfsr32(&prbsState);
  }

  size_t flashBase, flashSize;
  flash::getRange(&flashBase, &flashSize);
  size_t offset = flashBase + flashSize - TEST_SIZE;

  const uint8_t *mmapPtr;
  void *mmapHandle;
  *numErrors = 0;
  *resultCrc = 0;

  do {
    XMC_ERR_BRK(sts, flash::write(offset, buff, TEST_SIZE));
    XMC_ERR_BRK(sts, flash::mmap(offset, TEST_SIZE, &mmapHandle, &mmapPtr));

    for (size_t i = 0; i < TEST_SIZE; i++) {
      if (mmapPtr[i] != buff[i]) {
        (*numErrors)++;
      }
    }

    flash::munmap(mmapHandle);
  } while (false);

  xmcFree(buff);

  uint32_t testCrc = crc32(buff, TEST_SIZE);
  *resultCrc = crc32(mmapPtr, TEST_SIZE);
  if (testCrc != *resultCrc) {
    XMC_ERR_LOG(XMC_USER_GENERIC_ERROR);
  }

  if (*numErrors != 0) {
    XMC_ERR_LOG(XMC_USER_GENERIC_ERROR);
  }

  return sts;
}

uint32_t crc32(const uint8_t *data, size_t size) {
  uint32_t crc = 0xFFFFFFFF;
  for (size_t i = 0; i < size; i++) {
    uint8_t byte = data[i];
    for (int j = 0; j < 8; j++) {
      uint32_t bit = ((crc ^ byte) & 1);
      crc >>= 1;
      if (bit) {
        crc ^= 0xEDB88320;
      }
      byte >>= 1;
    }
  }
  return crc ^ 0xFFFFFFFF;
}
