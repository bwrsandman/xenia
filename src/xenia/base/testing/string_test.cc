/**
******************************************************************************
* Xenia : Xbox 360 Emulator Research Project                                 *
******************************************************************************
* Copyright 2019 Ben Vanik. All rights reserved.                             *
* Released under the BSD license - see LICENSE in the root for more details. *
******************************************************************************
*/

#include "xenia/base/string.h"

#include "third_party/catch/include/catch.hpp"

namespace xe {
namespace base {
namespace test {

TEST_CASE("find_base_path") {
  REQUIRE(find_base_path("C:\\Windows\\Users", '\\') == "C:\\Windows");
  REQUIRE(find_base_path(L"C:\\Windows\\Users", L'\\') == L"C:\\Windows");
  REQUIRE(find_base_path("C:\\Windows\\Users\\", '\\') == "C:\\Windows");
  REQUIRE(find_base_path(L"C:\\Windows\\Users\\", L'\\') == L"C:\\Windows");
  REQUIRE(find_base_path("C:\\Windows", '\\') == "C:\\");
  REQUIRE(find_base_path(L"C:\\Windows", L'\\') == L"C:\\");
  REQUIRE(find_base_path("C:\\", '\\') == "C:\\");
  REQUIRE(find_base_path(L"C:\\", L'\\') == L"C:\\");
  REQUIRE(find_base_path("C:/Windows/Users", '/') == "C:/Windows");
  REQUIRE(find_base_path(L"C:/Windows/Users", L'/') == L"C:/Windows");
  REQUIRE(find_base_path("C:\\Windows/Users", '/') == "C:\\Windows");
  REQUIRE(find_base_path(L"C:\\Windows/Users", L'/') == L"C:\\Windows");
  REQUIRE(find_base_path("C:\\Windows/Users", '\\') == "C:\\");
  REQUIRE(find_base_path(L"C:\\Windows/Users", L'\\') == L"C:\\");
  REQUIRE(find_base_path("/usr/bin/bash", '/') == "/usr/bin");
  REQUIRE(find_base_path(L"/usr/bin/bash", L'/') == L"/usr/bin");
  REQUIRE(find_base_path("/usr/bin", '/') == "/usr");
  REQUIRE(find_base_path(L"/usr/bin", L'/') == L"/usr");
  REQUIRE(find_base_path("/usr/bin/", '/') == "/usr");
  REQUIRE(find_base_path(L"/usr/bin/", L'/') == L"/usr");
  REQUIRE(find_base_path("/usr", '/') == "/");
  REQUIRE(find_base_path(L"/usr", L'/') == L"/");
  REQUIRE(find_base_path("/usr/", '/') == "/");
  REQUIRE(find_base_path(L"/usr/", L'/') == L"/");
  REQUIRE(find_base_path("~/filename.txt", '/') == "~");
  REQUIRE(find_base_path(L"~/filename.txt", L'/') == L"~");
  REQUIRE(find_base_path("local_dir/subdirectory", '/') == "local_dir");
  REQUIRE(find_base_path(L"local_dir/subdirectory", L'/') == L"local_dir");
  REQUIRE(find_base_path("local_dir", '/') == "");
  REQUIRE(find_base_path(L"local_dir", L'/') == L"");
}

}  // namespace test
}  // namespace base
}  // namespace xe
