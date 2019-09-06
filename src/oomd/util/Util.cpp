/*
 * Copyright (C) 2019-present, Facebook, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "oomd/util/Util.h"

#include <unistd.h>

#include <algorithm>
#include <sstream>

static constexpr auto kWhitespaceChars = " \t\n\r";

namespace {
void ltrim(std::string& s) {
  s.erase(0, s.find_first_not_of(kWhitespaceChars));
}

void rtrim(std::string& s) {
  s.erase(s.find_last_not_of(kWhitespaceChars) + 1);
}

template <class F, class T>
ssize_t wrapFull(F f, int fd, T buf, size_t count) {
  ssize_t totalBytes = 0;
  ssize_t r;
  do {
    r = f(fd, buf, count);
    if (r == -1) {
      if (errno == EINTR) {
        continue;
      }
      return r;
    }

    totalBytes += r;
    buf += r;
    count -= r;
  } while (r != 0 && count); // 0 means EOF

  return totalBytes;
}
} // namespace

namespace Oomd {

// "1.5G"    : 1.5 gigabytes - 1610612736 bytes
// "1G 128M" : 1 gigabyte and 128 megabytes - 1207959552 bytes
// "4K 2048" : 4 kilobytes and 2048 bytes - 6144 bytes
int Util::parseSize(const std::string& input, int64_t* output) {
  bool is_neg = false;
  uint64_t size = 0;
  size_t pos = 0;
  auto istr = input;

  // lower case and get rid of spaces
  transform(istr.begin(), istr.end(), istr.begin(), tolower);
  auto new_end = std::remove_if(istr.begin(), istr.end(), isspace);
  istr.erase(new_end - istr.begin());

  // pop off leading sign
  if (istr[0] == '+') {
    pos++;
  } else if (istr[0] == '-') {
    is_neg = true;
    pos++;
  }

  while (pos < istr.length()) {
    size_t unit_pos;
    size_t end_pos;

    unit_pos = istr.find_first_of("kmgt", pos);
    if (unit_pos == pos) {
      return -1;
    }
    if (unit_pos == std::string::npos) {
      unit_pos = istr.length();
    }

    auto num = istr.substr(pos, unit_pos - pos);
    auto unit = istr.c_str()[unit_pos];

    double v;
    try {
      v = std::stold(num, &end_pos);
    } catch (...) {
      return -1;
    }
    if (end_pos != num.length() || v < 0) {
      return -1;
    }

    switch (unit) {
      case '\0':
        break;
      case 'k':
        v *= 1ULL << 10;
        break;
      case 'm':
        v *= 1ULL << 20;
        break;
      case 'g':
        v *= 1ULL << 30;
        break;
      case 't':
        v *= 1ULL << 40;
        break;
      default:
        return -1;
    }
    size += v;
    pos = unit_pos + 1;
  }
  *output = is_neg ? -size : size;
  return 0;
}

std::vector<std::string> Util::split(const std::string& line, char delim) {
  std::istringstream iss(line);
  std::string item;
  std::vector<std::string> ret;
  while (std::getline(iss, item, delim)) {
    if (item.size()) {
      ret.push_back(std::move(item));
    }
  }
  return ret;
}

bool Util::startsWith(const std::string& prefix, const std::string& to_search) {
  if (prefix.size() > to_search.size()) {
    return false;
  }

  for (size_t i = 0; i < prefix.size(); ++i) {
    if (prefix[i] != to_search[i]) {
      return false;
    }
  }

  return true;
}

void Util::trim(std::string& s) {
  rtrim(s);
  ltrim(s);
}

ssize_t Util::readFull(int fd, char* msg_buf, size_t count) {
  return wrapFull(::read, fd, msg_buf, count);
}

ssize_t Util::writeFull(int fd, const char* msg_buf, size_t count) {
  return wrapFull(::write, fd, msg_buf, count);
}

} // namespace Oomd
