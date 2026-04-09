#ifndef XMC_BIOS_HPP
#define XMC_BIOS_HPP

#include "xmc/app.hpp"
#include "xmc/display.hpp"
#include "xmc/fs.hpp"
#include "xmc/gfx.hpp"
#include "xmc/input.hpp"
#include "xmc/system.hpp"

namespace xmc::diagnostic {

class App {
 public:
  static constexpr int MAX_FILES = (display::HEIGHT - 20) / 12;

  // Sprite frameBuffer = createSprite444(display::WIDTH, display::HEIGHT);
  fs::FileInfo fileList[MAX_FILES];
  int numFiles = 0;
  int fileCursor = 0;
  char currentDir[path::MAX_PATH_LENGTH + 1] = "/";

  static inline AppConfig getConfig() {
    AppConfig config = getDefaultAppConfig();
    config.displayPixelFormat = PixelFormat::RGB565;
    return config;
  }

  void setup() {
    fs::mount();
    updateFileList();
  }

  void loop() {
    // frameBuffer->completeTransfer();
    // frameBuffer->clear(0);
    //
    //// renderFileList();
    //
    // appDrawStatusBar(frameBuffer);
    // appDrawDebugInfo(frameBuffer);
    //
    // frameBuffer->startTransferToDisplay(0, 0);
  }

  void updateFileList() {
    numFiles = 0;
    fs::enumFiles(
        currentDir,
        [](const fs::FileInfo &info, void *userData) {
          App *app = static_cast<App *>(userData);
          if (app->numFiles < App::MAX_FILES) {
            app->fileList[app->numFiles++] = info;
          }
          return true;
        },
        this);
  }

  // void renderFileList() {
  //   int y = 12;
  //   for (int i = 0; i < numFiles; i++) {
  //     const fs::FileInfo &info = fileList[i];
  //     char buf[fs::MAX_FILENAME_LENGTH + 32];
  //     snprintf(buf, sizeof(buf), "%c %s (%lu bytes)",
  //              info.isDirectory ? 'D' : 'F', info.name, info.size);
  //     frameBuffer->setTextColor(i == fileCursor ? 0xF800 : 0xFFFF);
  //     frameBuffer->setFont(&ShapoSansP_s08c07, 1);
  //     frameBuffer->setCursor(4, y);
  //     frameBuffer->drawString(buf);
  //     y += 10;
  //   }
  // }
};

}  // namespace xmc::diagnostic

#endif
