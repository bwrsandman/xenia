/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2014 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/base/string.h"

// TODO(benvanik): when GCC finally gets codecvt, use that.
#if XE_PLATFORM_LINUX
#define NO_CODECVT 1
#else
#include <codecvt>
#endif  // XE_PLATFORM_LINUX

#include <cctype>
#include <cstring>
#include <locale>

namespace xe {

std::string to_string(const std::wstring& source) {
#if NO_CODECVT
  return std::string(source.begin(), source.end());
#else
  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.to_bytes(source);
#endif  // XE_PLATFORM_LINUX
}

std::wstring to_wstring(const std::string& source) {
#if NO_CODECVT
  return std::wstring(source.begin(), source.end());
#else
  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.from_bytes(source);
#endif  // XE_PLATFORM_LINUX
}

template <>
std::string format_string(const char* format, va_list args) {
  if (!format) {
    return "";
  }
  size_t max_len = 64;
  std::string new_s;
  while (true) {
    new_s.resize(max_len);
    int ret =
        std::vsnprintf(const_cast<char*>(new_s.data()), max_len, format, args);
    if (ret > max_len) {
      // Needed size is known (+2 for termination and avoid ambiguity).
      max_len = ret + 2;
    } else if (ret == -1 || ret >= max_len - 1) {
      // Handle some buggy vsnprintf implementations.
      max_len *= 2;
    } else {
      // Everything fit for sure.
      new_s.resize(ret);
      return new_s;
    }
  }
}

template <>
std::wstring format_string(const wchar_t* format, va_list args) {
  if (!format) {
    return L"";
  }
  size_t max_len = 64;
  std::wstring new_s;
  while (true) {
    new_s.resize(max_len);
    int ret = std::vswprintf(const_cast<wchar_t*>(new_s.data()), max_len,
                             format, args);
    if (ret > max_len) {
      // Needed size is known (+2 for termination and avoid ambiguity).
      max_len = ret + 2;
    } else if (ret == -1 || ret >= max_len - 1) {
      // Handle some buggy vsnprintf implementations.
      max_len *= 2;
    } else {
      // Everything fit for sure.
      new_s.resize(ret);
      return new_s;
    }
  }
}

std::string::size_type find_first_of_case(const std::string& target,
                                          const std::string& search) {
  const char* str = target.c_str();
  while (*str) {
    if (!strncasecmp(str, search.c_str(), search.size())) {
      break;
    }
    str++;
  }
  if (*str) {
    return str - target.c_str();
  } else {
    return std::string::npos;
  }
}

template <>
std::string to_absolute_path(const std::string& path) {
#if XE_PLATFORM_WIN32
  char buffer[kMaxPath];
  _fullpath(buffer, path.c_str(), sizeof(buffer) / sizeof(char));
  return buffer;
#else
  char buffer[kMaxPath];
  realpath(path.c_str(), buffer);
  return buffer;
#endif  // XE_PLATFORM_WIN32
}

template <>
std::wstring to_absolute_path(const std::wstring& path) {
#if XE_PLATFORM_WIN32
  wchar_t buffer[kMaxPath];
  _wfullpath(buffer, path.c_str(), sizeof(buffer) / sizeof(wchar_t));
  return buffer;
#else
  return xe::to_wstring(to_absolute_path(xe::to_string(path)));
#endif  // XE_PLATFORM_WIN32
}

int fuzzy_match(const std::string& pattern, const char* value) {
  // https://github.com/mattyork/fuzzy/blob/master/lib/fuzzy.js
  // TODO(benvanik): look at https://github.com/atom/fuzzaldrin/tree/master/src
  // This does not weight complete substrings or prefixes right, which
  // kind of sucks.
  size_t pattern_index = 0;
  size_t value_length = std::strlen(value);
  int total_score = 0;
  int local_score = 0;
  for (size_t i = 0; i < value_length; ++i) {
    if (std::tolower(value[i]) == std::tolower(pattern[pattern_index])) {
      ++pattern_index;
      local_score += 1 + local_score;
    } else {
      local_score = 0;
    }
    total_score += local_score;
  }
  return total_score;
}

std::vector<std::pair<size_t, int>> fuzzy_filter(const std::string& pattern,
                                                 const void* const* entries,
                                                 size_t entry_count,
                                                 size_t string_offset) {
  std::vector<std::pair<size_t, int>> results;
  results.reserve(entry_count);
  for (size_t i = 0; i < entry_count; ++i) {
    auto entry_value =
        reinterpret_cast<const char*>(entries[i]) + string_offset;
    int score = fuzzy_match(pattern, entry_value);
    results.emplace_back(i, score);
  }
  return results;
}

}  // namespace xe
