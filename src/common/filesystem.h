#pragma once

// Lightweight POSIX-based filesystem path abstraction for Linux/Android.
// Replaces the previous pathie-cpp dependency.

#include <string>
#include <stdexcept>
#include <cstddef>

namespace marian {
namespace filesystem {

  bool is_fifo(char const* path);
  bool is_fifo(std::string const& path);

  class Path {
    private:
      std::string path_;

    public:
      Path() {}
      Path(const Path& p) : path_{p.path_} {}
      Path& operator=(const Path& p) = default;
      Path(const std::string& s) : path_{s} {}

      Path parentPath() const;
      Path filename() const;
      Path extension() const;

      bool empty() const {
        return path_.empty();
      }

      operator std::string() const {
        return path_;
      }

      std::string string() const {
        return path_;
      }

      bool operator==(const Path& p) const {
        return path_ == p.path_;
      }

      bool operator!=(const Path& p) const {
        return path_ != p.path_;
      }

      // Internal helpers used by free functions
      Path absolute(const Path& base) const;
      Path absolute() const;
      Path expand() const;
      Path relative(const Path& base) const;
      bool exists() const;
      bool isDirectory() const;
      size_t size() const;

      friend Path operator/(const Path& lhs, const Path& rhs);
  };

  Path currentPathImpl(); // defined in filesystem.cpp

  static inline Path currentPath() {
    return currentPathImpl();
  }

  static inline Path canonical(const Path& p, const Path& base) {
    return p.absolute(base).expand();
  }

  static inline Path relative(const Path& p, const Path& base) {
    return p.absolute().expand().relative(base.absolute().expand());
  }

  static inline bool exists(const Path& p) {
    return p.exists();
  }

  static inline size_t fileSize(const Path& p) {
    return p.size();
  }

  static inline bool isDirectory(const Path& p) {
    return p.isDirectory();
  }

  Path operator/(const Path& lhs, const Path& rhs);

  using FilesystemError = std::runtime_error;

}  // namespace filesystem
}  // namespace marian
