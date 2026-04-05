#include "xmc/fs.hpp"
#include "xmc/hw/gpio.hpp"
#include "xmc/hw/pins.hpp"

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

XmcStatus init() {
  return XMC_OK;
}

XmcStatus deinit() {
  unmount();
  return XMC_OK;
}

XmcStatus mount() {
  if (mounted) return XMC_OK;

  FRESULT res = f_mount(&ffFs, "/", 0);
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
    strncpy(info.name, fno.fname, MAX_FILENAME_LENGTH);
    info.size = fno.fsize;
    info.isDirectory = (fno.fattrib & AM_DIR) != 0;

    cb(info, userData);
  }

  return XMC_OK;
}

size_t getSize(const char* path) {
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

    char childPath[MAX_PATH_LENGTH + 1];
    snprintf(childPath, MAX_PATH_LENGTH + 1, "%s/%s", path, fno.fname);

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

File::File(const char* path, FileMode mode) {
  size = getSize(path);

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

File::~File() { close(); }

bool File::isOpen() const { return handle != nullptr; }

XmcStatus File::close() {
  if (!handle) return XMC_OK;
  FRESULT res = f_close((FIL*)handle);
  if (res != FR_OK) {
    XMC_ERR_RET(XMC_ERR_FS_CLOSE_FAILED);
  }
  delete (FIL*)handle;
  handle = nullptr;
  return XMC_OK;
}

bool File::eof() const {
  if (!handle) return true;
  return position >= size;
}

size_t File::read(void* buffer, size_t size) {
  if (!handle) return 0;
  UINT bytesRead;
  FRESULT res = f_read((FIL*)handle, buffer, size, &bytesRead);
  if (res != FR_OK) {
    return 0;
  }
  position += bytesRead;
  return bytesRead;
}

size_t File::write(const void* buffer, size_t size) {
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

XmcStatus File::seek(int32_t offset, SeekOrigin origin) {
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

XmcStatus File::truncate(size_t size) {
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

}  // namespace xmc::fs
