/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2014 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_BASE_STRING_H_
#define XENIA_BASE_STRING_H_

#include <cstdarg>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

#include "xenia/base/platform.h"

namespace xe {

std::string to_string(const std::wstring& source);
std::wstring to_wstring(const std::string& source);

template <typename T>
std::basic_string<T> format_string(const T* format, va_list args);
template <typename T>
inline std::basic_string<T> format_string(const T* format, ...) {
  va_list va;
  va_start(va, format);
  auto result = format_string(format, va);
  va_end(va);
  return result;
}

// Splits the given string on any delimiters and returns all parts.
std::vector<std::string> split_string(const std::string& path,
                                      const std::string& delimiters);

std::vector<std::wstring> split_string(const std::wstring& path,
                                       const std::wstring& delimiters);

// find_first_of string, case insensitive.
std::string::size_type find_first_of_case(const std::string& target,
                                          const std::string& search);

// Converts the given path to an absolute path based on cwd.
template <typename T>
std::basic_string<T> to_absolute_path(const std::basic_string<T>& path);
template <typename T>
inline std::basic_string<T> to_absolute_path(const T* path) {
  return to_absolute_path(std::basic_string<T>(path));
}

// Splits the given path on any valid path separator and returns all parts.
template <typename T>
std::vector<std::basic_string<T>> split_path(const std::basic_string<T>& path) {
  std::vector<std::basic_string<T>> parts;
  size_t n = 0;
  size_t last = 0;
  while ((n = path.find_first_of(kAllPathSeparators<T>, last)) !=
         std::basic_string<T>::npos) {
    if (last != n) {
      parts.push_back(path.substr(last, n - last));
    }
    last = n + 1;
  }
  if (last != path.size()) {
    parts.push_back(path.substr(last));
  }
  return parts;
}
template <typename T>
inline std::vector<std::basic_string<T>> split_path(const T* path) {
  return split_path(std::basic_string<T>(path));
}

// Joins two path segments with the given separator.
template <typename T>
std::basic_string<T> join_paths(const std::basic_string<T>& left,
                                const std::basic_string<T>& right,
                                T sep = xe::kPathSeparator<T>) {
  if (left.empty()) {
    return right;
  } else if (right.empty()) {
    return left;
  }
  if (left[left.size() - 1] == sep) {
    return left + right;
  } else {
    return left + sep + right;
  }
}
template <typename T>
inline std::basic_string<T> join_paths(const std::basic_string<T>& left,
                                       const T* right,
                                       T sep = xe::kPathSeparator<T>) {
  return join_paths(left, std::basic_string<T>(right), sep);
}
template <typename T>
inline std::basic_string<T> join_paths(const T* left,
                                       const std::basic_string<T>& right,
                                       T sep = xe::kPathSeparator<T>) {
  return join_paths(std::basic_string<T>(left), right, sep);
}
template <typename T>
inline std::basic_string<T> join_paths(const T* left, const T* right,
                                       T sep = xe::kPathSeparator<T>) {
  return join_paths(std::basic_string<T>(left), std::basic_string<T>(right),
                    sep);
}

// Replaces all path separators with the given value and removes redundant
// separators.
template <typename T>
std::basic_string<T> fix_path_separators(const std::basic_string<T>& source,
                                         T new_sep = xe::kPathSeparator<T>) {
  // Swap all separators to new_sep.
  T old_sep = new_sep == kAllPathSeparators<T>[0] ? kAllPathSeparators<T>[1]
                                                  : kAllPathSeparators<T>[0];
  typename std::basic_string<T>::size_type pos = 0;
  std::basic_string<T> dest = source;
  while ((pos = source.find_first_of(old_sep, pos)) !=
         std::basic_string<T>::npos) {
    dest[pos] = new_sep;
    ++pos;
  }
  // Replace redundant separators.
  pos = 0;
  while ((pos = dest.find_first_of(new_sep, pos)) !=
         std::basic_string<T>::npos) {
    if (pos < dest.size() - 1) {
      if (dest[pos + 1] == new_sep) {
        dest.erase(pos + 1, 1);
      }
    }
    ++pos;
  }
  return dest;
}

template <typename T>
inline static std::basic_string<T> fix_path_separators(
    const T* source, T new_sep = xe::kPathSeparator<T>) {
  return fix_path_separators(std::basic_string<T>(source), new_sep);
}

// Find the top directory name or filename from a path.
template <typename T>
std::basic_string<T> find_name_from_path(const std::basic_string<T>& path,
                                         T sep = xe::kPathSeparator<T>) {
  std::basic_string<T> name(path);
  if (!path.empty()) {
    typename std::basic_string<T>::size_type from(std::basic_string<T>::npos);
    if (path.back() == sep) {
      if (path.length() == 1) {
        return path;
      }
      from = path.size() - 2;
    }
    auto pos(path.find_last_of(sep, from));
    if (pos != std::basic_string<T>::npos && pos != from) {
      if (from == std::basic_string<T>::npos) {
        name = path.substr(pos + 1);
      } else {
        auto len(from - pos);
        name = path.substr(pos + 1, len);
      }
    }
  }
  return name;
}
template <typename T>
inline std::basic_string<T> find_name_from_path(const T* path,
                                                T sep = xe::kPathSeparator<T>) {
  return find_name_from_path(std::basic_string<T>(path), sep);
}

// Get parent path of the given directory or filename.
template <typename T>
std::basic_string<T> find_base_path(const std::basic_string<T>& path,
                                    T sep = xe::kPathSeparator<T>) {
  auto path_start = path.find_first_of(T(':'));
  if (path_start == std::basic_string<T>::npos) {
    path_start = 0;  // Unix-like or local file
  } else {
    path_start++;  // Win32-like absolute path, i.e search after C:
  }
  auto last_slash = path.find_last_of(sep);
  if (last_slash == std::basic_string<T>::npos) {
    return std::basic_string<T>();
  } else if (last_slash == path_start) {  // Root directory
    return path.substr(0, path_start + 1);
  } else if (last_slash == path.length() - 1) {
    auto prev_slash = path.find_last_of(sep, last_slash - 1);
    if (prev_slash == std::basic_string<T>::npos) {
      return std::basic_string<T>();
    } else if (prev_slash == path_start) {
      return path.substr(0, path_start + 1);
    } else {
      return path.substr(0, prev_slash);
    }
  } else {
    return path.substr(0, last_slash);
  }
}
template <typename T>
inline std::basic_string<T> find_base_path(const T* path,
                                           T sep = xe::kPathSeparator<T>) {
  return find_base_path(std::basic_string<T>(path), sep);
}

// Tests a match against a case-insensitive fuzzy filter.
// Returns the score of the match or 0 if none.
int fuzzy_match(const std::string& pattern, const char* value);

// Applies a case-insensitive fuzzy filter to the given entries and ranks
// results.
// Entries is a list of pointers to opaque structs, each of which contains a
// char* string at the given offset.
// Returns an unsorted list of {original index, score}.
std::vector<std::pair<size_t, int>> fuzzy_filter(const std::string& pattern,
                                                 const void* const* entries,
                                                 size_t entry_count,
                                                 size_t string_offset);
template <typename T>
std::vector<std::pair<size_t, int>> fuzzy_filter(const std::string& pattern,
                                                 const std::vector<T>& entries,
                                                 size_t string_offset) {
  return fuzzy_filter(pattern, reinterpret_cast<void* const*>(entries.data()),
                      entries.size(), string_offset);
}

}  // namespace xe

#endif  // XENIA_BASE_STRING_H_
