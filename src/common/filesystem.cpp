#include "filesystem.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <climits>
#include <cstdlib>
#include <stdexcept>

namespace marian {
namespace filesystem {

bool is_fifo(char const* path) {
  struct stat buf;
  stat(path, &buf);
  return S_ISFIFO(buf.st_mode);
}

bool is_fifo(std::string const& path) {
  return is_fifo(path.c_str());
}

// Helper: remove trailing slashes (but keep root "/")
static std::string trimTrailingSlash(const std::string& p) {
  if(p.empty()) return p;
  size_t end = p.size();
  while(end > 1 && p[end - 1] == '/')
    --end;
  return p.substr(0, end);
}

Path Path::parentPath() const {
  std::string p = trimTrailingSlash(path_);
  if(p.empty()) return Path();
  auto pos = p.rfind('/');
  if(pos == std::string::npos)
    return Path(".");
  if(pos == 0)
    return Path("/");
  return Path(p.substr(0, pos));
}

Path Path::filename() const {
  std::string p = trimTrailingSlash(path_);
  if(p.empty()) return Path();
  auto pos = p.rfind('/');
  if(pos == std::string::npos)
    return Path(p);
  return Path(p.substr(pos + 1));
}

Path Path::extension() const {
  std::string name = filename().string();
  auto pos = name.rfind('.');
  if(pos == std::string::npos || pos == 0)
    return Path("");
  return Path(name.substr(pos));
}

Path Path::absolute(const Path& base) const {
  if(!path_.empty() && path_[0] == '/')
    return *this;
  std::string b = trimTrailingSlash(base.path_);
  if(b.empty())
    return *this;
  return Path(b + "/" + path_);
}

Path Path::absolute() const {
  if(!path_.empty() && path_[0] == '/')
    return *this;
  return absolute(currentPathImpl());
}

Path Path::expand() const {
  if(path_.empty()) return *this;
  char resolved[PATH_MAX];
  char* result = realpath(path_.c_str(), resolved);
  if(result)
    return Path(std::string(resolved));
  // If realpath fails (e.g., path doesn't exist), return as-is
  return *this;
}

Path Path::relative(const Path& base) const {
  // Simple implementation: if this path starts with base, strip it
  std::string p = trimTrailingSlash(path_);
  std::string b = trimTrailingSlash(base.path_);

  if(b.empty()) return *this;

  // Normalize: split into components and compute relative path
  // For simplicity, handle the common case where p starts with b
  if(p.size() > b.size() && p.substr(0, b.size()) == b && p[b.size()] == '/')
    return Path(p.substr(b.size() + 1));
  if(p == b)
    return Path(".");

  return *this;
}

bool Path::exists() const {
  struct stat buf;
  return stat(path_.c_str(), &buf) == 0;
}

bool Path::isDirectory() const {
  struct stat buf;
  if(stat(path_.c_str(), &buf) != 0)
    return false;
  return S_ISDIR(buf.st_mode);
}

size_t Path::size() const {
  struct stat buf;
  if(stat(path_.c_str(), &buf) != 0)
    return 0;
  return static_cast<size_t>(buf.st_size);
}

Path operator/(const Path& lhs, const Path& rhs) {
  if(lhs.path_.empty()) return rhs;
  if(rhs.path_.empty()) return lhs;
  std::string l = trimTrailingSlash(lhs.path_);
  return Path(l + "/" + rhs.path_);
}

Path currentPathImpl() {
  char buf[PATH_MAX];
  char* result = getcwd(buf, sizeof(buf));
  if(!result)
    throw std::runtime_error("Failed to get current working directory");
  return Path(std::string(buf));
}

} // end of namespace marian::filesystem
} // end of namespace marian
