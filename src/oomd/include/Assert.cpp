/*
 * Copyright (C) 2018-present, Facebook, Inc.
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

#include <features.h>
// backtrace(3) is a GNU extension
#ifdef __GNU_LIBRARY__
#include <execinfo.h>
#include <unistd.h>
#endif // __GNU_LIBRARY__

#include <cstdlib>
#include <iostream>

#include "oomd/include/Assert.h"

void __bt() {
#ifdef __GNU_LIBRARY__
  static const int num = 100;
  void* buffer[num];
  int n = backtrace(buffer, num);

  std::cerr << "Backtrace (most recent call first):" << std::endl;
  backtrace_symbols_fd(buffer, n, STDERR_FILENO);
#endif // __GNU_LIBRARY__
}

[[noreturn]] void
__OCHECK_FAIL(const char* expr, const char* file, int line, const char* func) {
  std::cerr << file << ":" << line << ": " << func << ": Assertion \'" << expr
            << "\' failed." << std::endl
            << std::flush;
  __bt();
  std::abort();
}
