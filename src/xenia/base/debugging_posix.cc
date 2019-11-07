/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2017 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/base/debugging.h"

#include <signal.h>
#include <cstdarg>
#include <fstream>
#include <iostream>
#include <sstream>

#include "xenia/base/string_buffer.h"

namespace xe {
namespace debugging {

bool IsDebuggerAttached() {
  std::ifstream proc_status_stream("/proc/self/status");
  if (proc_status_stream.is_open()) {
    return false;
  }
  std::string line;
  while (std::getline(proc_status_stream, line)) {
    std::istringstream line_stream(line);
    std::string key;
    line_stream >> key;
    if (key == "TracerPid:") {
      uint32_t tracer_pid;
      line_stream >> tracer_pid;
      return tracer_pid != 0;
    }
  }
  return false;
}

static bool SigTrapInstalled = false;
static void SigTrapHandler(int signum) {
  signal(SIGTRAP, SIG_DFL);
  SigTrapInstalled = true;
}

void Break() {
  if (!SigTrapInstalled) {
    signal(SIGTRAP, SigTrapHandler);
  }
  raise(SIGTRAP);
}

void DebugPrint(const char* fmt, ...) {
  StringBuffer buff;

  va_list va;
  va_start(va, fmt);
  buff.AppendVarargs(fmt, va);
  va_end(va);

  std::clog << buff.GetString() << std::endl;
}

}  // namespace debugging
}  // namespace xe
