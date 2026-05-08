#include "xmc/fs.hpp"
#include "xmc/flash.hpp"
#include "xmc/gpio.hpp"
#include "xmc/heap.hpp"
#include "xmc/pins.hpp"

#include <stdio.h>
#include <string.h>

#include <ff.h>
//----
#include <diskio.h>

#ifdef ARDUINO_ARCH_ESP32
typedef FF_DIR DIR;
#endif

namespace xmc::fs {

bool mounted = false;
FATFS ffFs;

XmcStatus init() { return XMC_OK; }

XmcStatus deinit() {
  unmount();
  return XMC_OK;
}

XmcStatus mount() {
  if (mounted) return XMC_OK;

  FRESULT res = f_mount(&ffFs, "/", 1);
  if (res != FR_OK) {
    XMC_ERR_RET(XMC_ERR_FS_MOUNT_FAILED);
  }

  mounted = true;
  return XMC_OK;
}
XmcStatus unmount() {
  if (!mounted) return XMC_OK;
  mounted = false;
  return XMC_OK;
}

XmcStatus enumFiles(const char* path, EnumFileCallback cb, void* userData) {
  DIR dir;
  FILINFO fno;

  if (!mounted) {
    XMC_ERR_RET(XMC_ERR_FS_NOT_MOUNTED);
  }

  FRESULT res = f_opendir(&dir, path);
  if (res != FR_OK) {
    XMC_ERR_RET(XMC_ERR_FS_OPEN_FAILED);
  }

  while (true) {
    res = f_readdir(&dir, &fno);
    if (res != FR_OK) {
      XMC_ERR_RET(XMC_ERR_FS_READ_FAILED);
    }
    if (fno.fname[0] == '\0') {
      break;  // end of directory
    }

    FileInfo info;
    strncpy(info.name, fno.fname, path::MAX_FILENAME_LENGTH);
    info.size = fno.fsize;
    info.isDirectory = (fno.fattrib & AM_DIR) != 0;

    if (!cb(info, userData)) {
      break;
    }
  }

  return XMC_OK;
}

XmcStatus enumFiles(const char* path, FileInfo* out, size_t maxFiles,
                    size_t* outNumFiles) {
  struct EnumData {
    FileInfo* out;
    size_t maxFiles;
    size_t numFiles;
  } data{out, maxFiles, 0};
  XmcStatus sts = enumFiles(
      path,
      [](const FileInfo& info, void* userData) {
        EnumData* data = (EnumData*)userData;
        if (data->numFiles < data->maxFiles) {
          data->out[data->numFiles++] = info;
          return true;
        }
        return false;
      },
      &data);
  if (sts != XMC_OK) {
    XMC_ERR_RET(sts);
  }
  if (outNumFiles) {
    *outNumFiles = data.numFiles;
  }
  return XMC_OK;
}

size_t getFileSize(const char* path) {
  FILINFO fi;
  FRESULT res = f_stat(path, &fi);
  if (res != FR_OK) return 0;
  return fi.fsize;
}

bool isDirectory(const char* path) {
  FILINFO fi;
  FRESULT res = f_stat(path, &fi);
  if (res != FR_OK) return false;
  return (fi.fattrib & AM_DIR) != 0;
}

bool exists(const char* path) {
  FILINFO fi;
  FRESULT res = f_stat(path, &fi);
  return res == FR_OK;
}

XmcStatus createDirectory(const char* path) {
  FRESULT res = f_mkdir(path);
  if (res != FR_OK) {
    XMC_ERR_RET(XMC_ERR_FS_OPERATION_FAILED);
  }
  return XMC_OK;
}

XmcStatus removeDirectory(const char* path) {
  DIR dir;
  FILINFO fno;

  FRESULT res = f_opendir(&dir, path);
  if (res != FR_OK) {
    XMC_ERR_RET(XMC_ERR_FS_OPEN_FAILED);
  }

  while (true) {
    res = f_readdir(&dir, &fno);
    if (res != FR_OK) {
      XMC_ERR_RET(XMC_ERR_FS_READ_FAILED);
    }
    if (fno.fname[0] == '\0') {
      break;  // end of directory
    }

    char childPath[path::MAX_PATH_LENGTH + 1];
    snprintf(childPath, path::MAX_PATH_LENGTH + 1, "%s/%s", path, fno.fname);

    if ((fno.fattrib & AM_DIR) != 0) {
      // Directory
      XmcStatus sts = removeDirectory(childPath);
      if (sts != XMC_OK) {
        XMC_ERR_RET(sts);
      }
    } else {
      // File
      res = f_unlink(childPath);
      if (res != FR_OK) {
        XMC_ERR_RET(XMC_ERR_FS_OPERATION_FAILED);
      }
    }
  }

  res = f_unlink(path);
  if (res != FR_OK) {
    XMC_ERR_RET(XMC_ERR_FS_OPERATION_FAILED);
  }
  return XMC_OK;
}

XmcStatus removeFile(const char* path) {
  FRESULT res = f_unlink(path);
  if (res != FR_OK) {
    XMC_ERR_RET(XMC_ERR_FS_OPERATION_FAILED);
  }
  return XMC_OK;
}

FileClass::FileClass(const char* path, FileMode mode) {
  size = getFileSize(path);

  handle = new FIL;
  BYTE fattrib = 0;
  if ((mode & FileMode::READ) != FileMode(0)) fattrib |= FA_READ;
  if ((mode & FileMode::WRITE) != FileMode(0)) fattrib |= FA_WRITE;
  if ((mode & FileMode::APPEND) != FileMode(0)) fattrib |= FA_OPEN_APPEND;

  FRESULT res = f_open((FIL*)handle, path, fattrib);
  if (res != FR_OK) {
    delete (FIL*)handle;
    handle = nullptr;
  }
}

FileClass::~FileClass() { close(); }

bool FileClass::isOpen() const { return handle != nullptr; }

XmcStatus FileClass::close() {
  if (!handle) return XMC_OK;
  FRESULT res = f_close((FIL*)handle);
  if (res != FR_OK) {
    XMC_ERR_RET(XMC_ERR_FS_CLOSE_FAILED);
  }
  delete (FIL*)handle;
  handle = nullptr;
  return XMC_OK;
}

bool FileClass::eof() const {
  if (!handle) return true;
  return position >= size;
}

size_t FileClass::read(void* buffer, size_t size) {
  if (!handle) return 0;
  UINT bytesRead;
  FRESULT res = f_read((FIL*)handle, buffer, size, &bytesRead);
  if (res != FR_OK) {
    return 0;
  }
  position += bytesRead;
  return bytesRead;
}

size_t FileClass::write(const void* buffer, size_t size) {
  if (!handle) return 0;
  UINT bytesWritten;
  FRESULT res = f_write((FIL*)handle, buffer, size, &bytesWritten);
  if (res != FR_OK) {
    return 0;
  }
  position += bytesWritten;
  if (position > this->size) {
    this->size = position;
  }
  return bytesWritten;
}

XmcStatus FileClass::seek(int32_t offset, SeekOrigin origin) {
  if (!handle) XMC_ERR_RET(XMC_ERR_FS_NOT_OPENED);
  FSIZE_t newPos;
  switch (origin) {
    case SeekOrigin::BEGIN: newPos = offset; break;
    case SeekOrigin::CURRENT: newPos = position + offset; break;
    case SeekOrigin::END: newPos = size + offset; break;
    default: XMC_ERR_RET(XMC_ERR_BAD_PARAMETER);
  }
  FRESULT res = f_lseek((FIL*)handle, newPos);
  if (res != FR_OK) {
    XMC_ERR_RET(XMC_ERR_FS_SEEK_FAILED);
  }
  position = newPos;
  return XMC_OK;
}

XmcStatus FileClass::truncate(size_t size) {
  if (!handle) XMC_ERR_RET(XMC_ERR_FS_NOT_OPENED);
  FRESULT res = f_truncate((FIL*)handle);
  if (res != FR_OK) {
    XMC_ERR_RET(XMC_ERR_FS_OPERATION_FAILED);
  }
  this->size = size;
  if (position > size) {
    position = size;
  }
  return XMC_OK;
}

XmcStatus copyFileToFlash(const char* srcPath, size_t srcOffset, size_t size,
                          size_t destOffset, size_t bufferSize) {
  auto file = fs::openFile(srcPath, fs::FileMode::READ);
  if (!file) {
    XMC_ERR_RET(XMC_ERR_FS_OPEN_FAILED);
  }

  if (bufferSize == 0) {
    bufferSize = flash::getSectorSize();
  }

  XmcStatus sts = XMC_OK;
  uint8_t* buffer =
      static_cast<uint8_t*>(xmcMalloc(bufferSize, XMC_HEAP_CAP_SPIRAM));
  if (!buffer) {
    XMC_ERR_RET(XMC_ERR_RAM_ALLOC_FAILED);
  }

  do {
    XMC_ERR_BRK(sts, file->seek(srcOffset));

    XMC_ERR_BRK(sts, flash::erase(destOffset, size));

    while (size > 0) {
      size_t chunkSize = std::min(size, bufferSize);
      size_t bytesRead = file->read(buffer, chunkSize);
      if (bytesRead != chunkSize) {
        XMC_ERR_BRK(sts, XMC_ERR_FS_READ_FAILED);
      }
      XMC_ERR_BRK(sts, flash::write(destOffset, buffer, chunkSize));
      destOffset += chunkSize;
      srcOffset += chunkSize;
      size -= chunkSize;
    }
  } while (false);
  file->close();
  xmcFree(buffer);
  return sts;
}

}  // namespace xmc::fs
