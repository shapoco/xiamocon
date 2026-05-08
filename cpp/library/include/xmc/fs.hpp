#ifndef XMC_FS_HPP
#define XMC_FS_HPP

#include "xmc/path.hpp"
#include "xmc/xmc_common.hpp"

#include <memory>

namespace xmc::fs {

/** Information about a file or directory. */
struct FileInfo {
  /** Name of the file or directory. */
  char name[path::MAX_FILENAME_LENGTH + 1];

  /** Size of the file in bytes. */
  uint32_t size;

  /** Whether the entry is a directory. */
  bool isDirectory;
};

/** File access modes. */
enum class FileMode : int {
  READ = (1 << 0),
  WRITE = (1 << 1),
  APPEND = (1 << 2)
};
XMC_ENUM_FLAGS(FileMode, int)

/** Seek origin for file operations. */
enum class SeekOrigin : int { BEGIN = 0, CURRENT = 1, END = 2 };

/** Callback function type for enumerating files. */
using EnumFileCallback = bool (*)(const FileInfo& info, void* userData);

/**
 * Initialize the filesystem module.
 * @return XMC_OK on success, or an error code on failure.
 */
XmcStatus init();

/**
 * Deinitialize the filesystem module.
 * @return XMC_OK on success, or an error code on failure.
 */
XmcStatus deinit();

/**
 * Mount the filesystem.
 * @return XMC_OK on success, or an error code on failure.
 */
XmcStatus mount();

/**
 * Unmount the filesystem.
 * @return XMC_OK on success, or an error code on failure.
 */
XmcStatus unmount();

/**
 * Enumerate files in a directory.
 * @param path The path of the directory to enumerate.
 * @param cb The callback function to call for each file or directory found.
 * @param userData User data to pass to the callback function.
 * @return XMC_OK on success, or an error code on failure.
 */
XmcStatus enumFiles(const char* path, EnumFileCallback cb, void* userData);

/**
 * Enumerate files in a directory and store the results in an output array.
 * @param path The path of the directory to enumerate.
 * @param out The output array to store the file information.
 * @param maxFiles The maximum number of files to store in the output array.
 * @param outNumFiles A pointer to a variable to receive the actual number of
 * files found and stored in the output array.
 * @return XMC_OK on success, or an error code on failure.
 */
XmcStatus enumFiles(const char* path, FileInfo* out, size_t maxFiles,
                    size_t* outNumFiles);

/**
 * Get the size of a file.
 * @param path The path of the file.
 * @return The size of the file in bytes, or 0 if the file does not exist or is
 * a directory.
 */
size_t getFileSize(const char* path);

/**
 * Check if a path is a directory.
 * @param path The path to check.
 * @return true if the path is a directory, false otherwise.
 */
bool isDirectory(const char* path);
bool exists(const char* path);

/**
 * Create a directory.
 * @param path The path of the directory to create.
 * @return XMC_OK on success, or an error code on failure.
 */
XmcStatus createDirectory(const char* path);

/**
 * Remove a directory.
 * @param path The path of the directory to remove.
 * @return XMC_OK on success, or an error code on failure.
 */
XmcStatus removeDirectory(const char* path);

/**
 * Remove a file.
 * @param path The path of the file to remove.
 * @return XMC_OK on success, or an error code on failure.
 */
XmcStatus removeFile(const char* path);

/**
 * Copy a file from the filesystem to flash memory.
 * @param srcPath The path of the source file in the filesystem.
 * @param srcOffset The offset in the source file to start copying from, in
 * bytes.
 * @param size The number of bytes to copy.
 * @param destOffset The offset in flash memory to write the data to, in bytes.
 * @param bufferSize The size of the buffer to use for copying, in bytes. If 0,
 * a default size will be used.
 * @return XMC_OK on success, or an error code on failure.
 */
XmcStatus copyFileToFlash(const char* srcPath, size_t srcOffset, size_t size,
                          size_t destOffset, size_t bufferSize = 0);

/**
 * Class representing an open file. The file is automatically closed when the
 * object is destroyed.
 */
class FileClass {
 private:
  void* handle = nullptr;
  bool opened = false;
  size_t size = 0;
  size_t position = 0;

 public:
  /**
   * Open a file with the specified path and mode.
   * @param path The path of the file to open.
   * @param mode The access mode for the file (read, write, append).
   */
  FileClass(const char* path, FileMode mode);

  /**
   * Destructor that closes the file if it is still open.
   */
  ~FileClass();

  /**
   * Get the size of the file in bytes.
   * @return The size of the file in bytes, or 0 if the file is not open.
   */
  inline size_t getSize() const { return size; }

  /** @return true if the file is open, false otherwise. */
  bool isOpen() const;

  /**
   * Close the file if it is open.
   * @return XMC_OK on success, or an error code on failure.
   */
  XmcStatus close();

  /**
   * @return true if the end of the file has been reached, false otherwise.
   * If the file is not open, this function returns true.
   */
  bool eof() const;

  /**
   * Read data from the file into the provided buffer.
   * @param buffer The buffer to read data into.
   * @param size The number of bytes to read.
   * @return The number of bytes actually read, or 0 on failure.
   */
  size_t read(void* buffer, size_t size);

  /**
   * Write data to the file from the provided buffer.
   * @param buffer The buffer containing the data to write.
   * @param size The number of bytes to write.
   * @return The number of bytes actually written, or 0 on failure.
   */
  size_t write(const void* buffer, size_t size);

  /**
   * Move the file pointer to a new position based on the specified offset and
   * origin.
   * @param offset The offset to move the file pointer by, in bytes.
   * @param origin The reference point for the offset (beginning, current
   * position, or end of the file).
   * @return XMC_OK on success, or an error code on failure.
   */
  XmcStatus seek(int32_t offset, SeekOrigin origin = SeekOrigin::BEGIN);

  /**
   * Truncate the file to the specified size. If the file is larger than the
   * specified size, the extra data will be lost. If the file is smaller than
   * the specified size, it will be extended and the new space will be filled
   * with zeros.
   * @param size The new size of the file in bytes.
   * @return XMC_OK on success, or an error code on failure.
   */
  XmcStatus truncate(size_t size);
};

using File = std::shared_ptr<FileClass>;

static inline File openFile(const char* path, FileMode mode) {
  return std::make_shared<FileClass>(path, mode);
}

}  // namespace xmc::fs

#endif
