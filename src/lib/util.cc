#include "util.h"

#include <sys/stat.h>
#include <sys/types.h>

#include <cassert>
#include <cstring>

namespace {

const std::string whitespace = " \f\n\r\t\v";

} // namespace

std::vector<std::string> split(std::string in, char delim) {
  std::vector<std::string> items;
  std::istringstream ss(in);
  std::string item;
  while (std::getline(ss, item, delim)) {
    items.push_back(item);
  }
  return items;
}

std::string trimLeft(const std::string& in) {
  return in.substr(in.find_first_not_of(whitespace));
}

std::string trimRight(const std::string& in) {
  return in.substr(0, in.find_last_not_of(whitespace) + 1);
}

bool parseTime(const std::string& in, struct timespec* ts) {
  const char* buf = in.c_str();
  struct tm tm;
  long int tv_nsec = 0;

  // For example: 2017-12-21T17:46:32.2Z
  char* pos = strptime(buf, "%Y-%m-%dT%H:%M:%S", &tm);
  if (pos < (buf + in.size())) {
    if (pos[0] == '.') {
      // Expect single decimal for fractional second
      int dec = 0;
      int num = sscanf(pos, ".%dZ", &dec);
      if (num == 1 && dec < 10) {
        ts->tv_nsec = num * 100000000;
      } else {
        return false;
      }
    } else {
      return false;
    }
  }

  auto t = mktime(&tm);
  ts->tv_sec = t;
  ts->tv_nsec = tv_nsec;
  return true;
}

void mkdirp(const std::string& path) {
  size_t pos = 0;

  for (;; pos++) {
    pos = path.find('/', pos);
    if (pos == 0) {
      continue;
    }
    auto sub = path.substr(0, pos);
    auto rv = mkdir(sub.c_str(), S_IRWXU);
    if (rv == -1 && errno != EEXIST) {
      perror("mkdir");
      assert(rv == 0);
    }
    if (pos == std::string::npos) {
      break;
    }
  }
}
