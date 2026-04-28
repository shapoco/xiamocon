#include <xiamocon.hpp>

#include "icon16_file.hpp"
#include "icon16_folder.hpp"

using namespace xmc;
using namespace xmc::input;

static constexpr PixelFormat DISPLAY_FORMAT = PixelFormat::RGB565;
static constexpr int MAX_NUM_ITEMS = 128;
static constexpr int ADDR_BAR_TOP = STATUS_BAR_HEIGHT;
static constexpr int ADDR_BAR_HEIGHT = 16;
static constexpr int TEXT_Y_OFFSET = 3;

FrameBuffer frameBuffer = createFrameBuffer(DISPLAY_FORMAT, false);
bool renderRequested = false;
uint64_t lastRenderUs = 0;

Sprite icon16folder = createIcon16FolderSprite();
Sprite icon16file = createIcon16FileSprite();

char currentDirectory[path::MAX_PATH_LENGTH + 1] = "/";

class FileList {
 public:
  static constexpr int TOP = ADDR_BAR_TOP + ADDR_BAR_HEIGHT;
  static constexpr int HEIGHT = display::HEIGHT - TOP;
  static constexpr int ITEM_HEIGHT = 20;

  int numFiles = 0;
  fs::FileInfo files[MAX_NUM_ITEMS];
  bool updateRequested = false;
  bool renderRequested = false;
  int selectedIndex = 0;

  int scrollPosGoal = 0;
  int scrollPos = 0;

  void requestUpdate() { updateRequested = true; }
  void requestRender() { renderRequested = true; }

  bool keyInput() {
    XmcStatus sts = XMC_OK;
    if (wasPressed(Button::UP)) {
      if (numFiles > 0) {
        selectItem((selectedIndex - 1 + numFiles) % numFiles);
        return true;
      }
    } else if (wasPressed(Button::DOWN)) {
      if (numFiles > 0) {
        selectItem((selectedIndex + 1) % numFiles);
        return true;
      }
    } else if (wasPressed(Button::A)) {
      if (0 <= selectedIndex && selectedIndex < numFiles) {
        const fs::FileInfo &info = files[selectedIndex];
        if (info.isDirectory) {
          do {
            if (strcmp(info.name, "..") == 0) {
              // Special case for "..": go up to parent directory
              XMC_ERR_BRK(sts, path::dirname(currentDirectory, currentDirectory,
                                             sizeof(currentDirectory)));
              if (strnlen(currentDirectory, sizeof(currentDirectory)) == 0) {
                strncpy(currentDirectory, "/", sizeof(currentDirectory));
              }
            } else {
              XMC_ERR_BRK(sts, path::join(currentDirectory, info.name));
            }
            update(true);
            selectItem(0);
          } while (false);
          return true;
        }
      }
    }
    return false;
  }

  void selectItem(int index) {
    if (0 <= index && index < numFiles) {
      selectedIndex = index;
      scrollTo(selectedIndex);
      requestRender();
    }
  }

  void scrollTo(int index) {
    int itemTop = index * ITEM_HEIGHT;
    int itemBottom = itemTop + ITEM_HEIGHT;
    if (itemTop < scrollPosGoal) {
      scrollPosGoal = itemTop;
      requestRender();
    } else if (itemBottom > scrollPosGoal + HEIGHT) {
      scrollPosGoal = itemBottom - HEIGHT;
      requestRender();
    }
  }

  XmcStatus update(bool force = false) {
    if (!updateRequested && !force) return XMC_OK;
    updateRequested = false;

    XmcStatus sts = XMC_OK;
    numFiles = 0;

    do {
      fs::FileInfo *f = files;
      size_t max = (size_t)MAX_NUM_ITEMS;

      if (currentDirectory[0] != '/' || currentDirectory[1] != '\0') {
        // Special case for root directory: add ".." entry
        f->isDirectory = true;
        strncpy(f->name, "..", path::MAX_FILENAME_LENGTH);
        f++;
        max--;
      }

      size_t n;
      XMC_ERR_BRK(sts, fs::enumFiles(currentDirectory, f, max, &n));
      numFiles = n;
      if (selectedIndex >= numFiles) {
        selectedIndex = numFiles - 1;
        scrollTo(selectedIndex);
      }
    } while (false);
    requestRender();
    return sts;
  }

  XmcStatus render(Graphics2D &gfx) {
    renderRequested = false;

    gfx->setFont(&ShapoSansP_s12c09a01w02, 1);
    gfx->setTextColor(0xFFFF);

    gfx->setClipRect(0, TOP, display::WIDTH, HEIGHT);
    int y = TOP - scrollPos;
    for (int i = 0; i < numFiles; i++) {
      if (y >= TOP - ITEM_HEIGHT) {
        const fs::FileInfo &info = files[i];
        uint16_t bgColor = (i == selectedIndex) ? clipPack565(0, 24, 24) : 0x0000;
        gfx->fillRect(0, y, display::WIDTH, ITEM_HEIGHT, bgColor);
        Sprite icon = info.isDirectory ? icon16folder : icon16file;
        gfx->drawImage(icon, 10, y + (ITEM_HEIGHT - 16) / 2, 16, 16, 0, 0);
        gfx->setCursor(30, y + TEXT_Y_OFFSET);
        gfx->drawString(info.name);
      }
      y += ITEM_HEIGHT;
      if (y > TOP + HEIGHT) {
        break;
      }
    }
    gfx->clearClipRect();

    if (scrollPos != scrollPosGoal) {
      int d = scrollPosGoal - scrollPos;
      if (d > 0) {
        d = (d + 3) / 4;
      } else {
        d = (d - 3) / 4;
      }
      scrollPos += d;
      requestRender();
    }

    return XMC_OK;
  }
};
FileList fileList;

static inline void requestRenderAll() {
  renderRequested = true;
  fileList.requestRender();
}
XmcStatus render();
XmcStatus renderAddressBar(Graphics2D &gfx);

AppConfig xmcAppGetConfig(void) {
  AppConfig cfg = getDefaultAppConfig();
  cfg.displayPixelFormat = DISPLAY_FORMAT;
  cfg.speakerEnabled = false;
  return cfg;
}

void xmcAppSetup(void) {
  fs::mount();
  fileList.requestUpdate();
  requestRenderAll();
  frameBuffer->enableFlag(FrameBufferFlags::SHOW_DEBUG_INFO);
}

void xmcAppLoop(void) {
  uint64_t nowUs = getTimeUs();

  fileList.keyInput();

  fileList.update();

  if (nowUs >= lastRenderUs + 1000000) {
    requestRenderAll();
  }

  if (renderRequested || fileList.renderRequested) {
    render();
    lastRenderUs = getTimeUs();
  }
}

XmcStatus render() {
  renderRequested = false;
  frameBuffer->beginRender();

  Graphics2D gfx = frameBuffer->createGraphics();
  gfx->clear(0x0000);

  renderAddressBar(gfx);
  fileList.render(gfx);

  frameBuffer->endRender();
  return XMC_OK;
}

XmcStatus renderAddressBar(Graphics2D &gfx) {
  gfx->setFont(&ShapoSansP_s12c09a01w02, 1);
  gfx->setTextColor(0xFFFF);
  gfx->fillRect(0, ADDR_BAR_TOP, display::WIDTH, ADDR_BAR_HEIGHT,
                clipPack565(16, 32, 16));
  gfx->setCursor(5, ADDR_BAR_TOP + TEXT_Y_OFFSET);
  gfx->drawString(currentDirectory);
  return XMC_OK;
}
