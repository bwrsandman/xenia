/**
******************************************************************************
* Xenia : Xbox 360 Emulator Research Project                                 *
******************************************************************************
* Copyright 2019 Ben Vanik. All rights reserved.                             *
* Released under the BSD license - see LICENSE in the root for more details. *
******************************************************************************
*/

#include "xenia/base/string.h"
#include "xenia/base/string_buffer.h"

#include "third_party/catch/include/catch.hpp"

namespace xe {
namespace base {
namespace test {

TEST_CASE("find_name_from_path", "string") {
  REQUIRE(find_name_from_path("", '\\') == "");
  REQUIRE(find_name_from_path(L"", L'\\') == L"");
  REQUIRE(find_name_from_path("xe:\\\\xam.xex", '\\') == "xam.xex");
  REQUIRE(find_name_from_path(L"xe:\\\\xam.xex", L'\\') == L"xam.xex");
  REQUIRE(find_name_from_path("xe:\\\\", '\\') == "xe:\\\\");
  REQUIRE(find_name_from_path(L"xe:\\\\", L'\\') == L"xe:\\\\");
  REQUIRE(find_name_from_path("C:\\filename.txt", '\\') == "filename.txt");
  REQUIRE(find_name_from_path(L"C:\\filename.txt", L'\\') == L"filename.txt");
  REQUIRE(find_name_from_path("C:\\Windows\\Users\\filename.txt", '\\') ==
          "filename.txt");
  REQUIRE(find_name_from_path(L"C:\\Windows\\Users\\filename.txt", L'\\') ==
          L"filename.txt");
  REQUIRE(find_name_from_path("C:\\", '\\') == "C:\\");
  REQUIRE(find_name_from_path(L"C:\\", L'\\') == L"C:\\");
  REQUIRE(find_name_from_path("C:\\Windows\\Users/filename.txt", '\\') ==
          "Users/filename.txt");
  REQUIRE(find_name_from_path(L"C:\\Windows\\Users/filename.txt", L'\\') ==
          L"Users/filename.txt");
  REQUIRE(find_name_from_path("C:/Windows/Users/filename.txt", '/') ==
          "filename.txt");
  REQUIRE(find_name_from_path(L"C:/Windows/Users/filename.txt", L'/') ==
          L"filename.txt");
  REQUIRE(find_name_from_path("/", '/') == "/");
  REQUIRE(find_name_from_path(L"/", L'/') == L"/");
  REQUIRE(find_name_from_path("/usr", '/') == "usr");
  REQUIRE(find_name_from_path(L"/usr", L'/') == L"usr");
  REQUIRE(find_name_from_path("~/filename.txt", '/') == "filename.txt");
  REQUIRE(find_name_from_path(L"~/filename.txt", L'/') == L"filename.txt");
  REQUIRE(find_name_from_path("~/.local/share/xenia/filename.txt", '/') ==
          "filename.txt");
  REQUIRE(find_name_from_path(L"~/.local/share/xenia/filename.txt", L'/') ==
          L"filename.txt");
  REQUIRE(find_name_from_path("~/.local/share/xenia", '/') == "xenia");
  REQUIRE(find_name_from_path(L"~/.local/share/xenia", L'/') == L"xenia");
  REQUIRE(find_name_from_path("~/.local/share/xenia/", '/') == "xenia");
  REQUIRE(find_name_from_path(L"~/.local/share/xenia/", L'/') == L"xenia");
  REQUIRE(find_name_from_path("filename.txt", '/') == "filename.txt");
  REQUIRE(find_name_from_path(L"filename.txt", L'/') == L"filename.txt");
  REQUIRE(find_name_from_path("C:\\New Volume\\filename.txt", '\\') ==
          "filename.txt");
  REQUIRE(find_name_from_path(L"C:\\New Volume\\filename.txt", L'\\') ==
          L"filename.txt");
  REQUIRE(find_name_from_path("C:\\New Volume\\dir\\filename.txt", '\\') ==
          "filename.txt");
  REQUIRE(find_name_from_path(L"C:\\New Volume\\dir\\filename.txt", L'\\') ==
          L"filename.txt");
  REQUIRE(find_name_from_path("C:\\New Volume\\file name.txt", '\\') ==
          "file name.txt");
  REQUIRE(find_name_from_path(L"C:\\New Volume\\file name.txt", L'\\') ==
          L"file name.txt");
  REQUIRE(find_name_from_path("/home/xenia/New Volume/file name.txt", '/') ==
          "file name.txt");
  REQUIRE(find_name_from_path(L"/home/xenia/New Volume/file name.txt", L'/') ==
          L"file name.txt");
}

TEST_CASE("find_base_path", "string") {
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
  REQUIRE(find_base_path("C:\\New Volume\\filename.txt", '\\') ==
          "C:\\New Volume");
  REQUIRE(find_base_path(L"C:\\New Volume\\filename.txt", L'\\') ==
          L"C:\\New Volume");
  REQUIRE(find_base_path("C:\\New Volume\\dir\\filename.txt", '\\') ==
          "C:\\New Volume\\dir");
  REQUIRE(find_base_path(L"C:\\New Volume\\dir\\filename.txt", L'\\') ==
          L"C:\\New Volume\\dir");
  REQUIRE(find_base_path("C:\\New Volume\\file name.txt", '\\') ==
          "C:\\New Volume");
  REQUIRE(find_base_path(L"C:\\New Volume\\file name.txt", L'\\') ==
          L"C:\\New Volume");
  REQUIRE(find_base_path("/home/xenia/New Volume/file name.txt", '/') ==
          "/home/xenia/New Volume");
  REQUIRE(find_base_path(L"/home/xenia/New Volume/file name.txt", L'/') ==
          L"/home/xenia/New Volume");
}

TEST_CASE("StringBuffer") {
  StringBuffer sb;
  uint32_t module_flags = 0x1000000;

  std::string path_(R"(\Device\Cdrom0\default.xex)");
  sb.AppendFormat("Module %s:\n", path_.c_str());
  REQUIRE(sb.to_string() == "Module \\Device\\Cdrom0\\default.xex:\n");
  sb.AppendFormat("    Module Flags: %.8X\n", module_flags);
  REQUIRE(
      sb.to_string() ==
      "Module \\Device\\Cdrom0\\default.xex:\n    Module Flags: 01000000\n");
}

}  // namespace test
}  // namespace base
}  // namespace xe
