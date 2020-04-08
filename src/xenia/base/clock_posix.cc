/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2019 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include <sys/time.h>

#include "xenia/base/assert.h"
#include "xenia/base/clock.h"

namespace xe {

uint64_t Clock::host_tick_frequency_platform() {
  timespec res;
  auto error = clock_getres(CLOCK_MONOTONIC_RAW, &res);
  assert(!error);
  assert(!res.tv_sec);

  // Convert nano seconds to hertz. Resolution is usually 1ns on most systems.
  return 1000000000ull / res.tv_nsec;
}

uint64_t Clock::host_tick_count_platform() {
  timespec tp;
  auto error = clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
  assert(!error);

  return tp.tv_nsec + tp.tv_sec * 1000000000ull;
}

uint64_t Clock::QueryHostSystemTime() {
  constexpr uint64_t seconds_per_day = 3600 * 24;
  // Don't forget the 89 leap days.
  constexpr uint64_t seconds_1601_to_1970 =
      ((369 * 365 + 89) * seconds_per_day);

  timeval now;
  auto error = gettimeofday(&now, nullptr);
  assert(!error);
  assert(sizeof(now.tv_sec) == 8);

  // NT systems use 100ns intervals.
  return (now.tv_sec + seconds_1601_to_1970) * 10000000ull + now.tv_usec * 10;
}

uint64_t Clock::QueryHostUptimeMillis() {
  return host_tick_count_platform() * 1000 / host_tick_frequency_platform();
}

}  // namespace xe