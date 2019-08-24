/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2019 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/base/filesystem.h"

#include "third_party/catch/include/catch.hpp"

namespace xe {
namespace base {
namespace test {

TEST_CASE("file_get_info", "Get Info") {
  auto test_file_name = L"test_file";
  auto test_file_dir = xe::fix_path_separators(L"src/xenia/base/testing/res");
  auto test_file_path = xe::join_paths(test_file_dir, test_file_name);

  filesystem::FileInfo info = {};

  REQUIRE(filesystem::GetInfo(test_file_path, &info));

  CHECK(info.type == filesystem::FileInfo::Type::kFile);
  CHECK(info.name == test_file_name);
  CHECK(info.path == test_file_dir);
  CHECK(info.total_size == 81);
  CHECK(info.create_timestamp > 132111406279379842);
  CHECK(info.access_timestamp > 132111406279379842);
  CHECK(info.write_timestamp > 132111406279379842);
}

TEST_CASE("folder_get_info", "Get Info") {
  auto test_folder_name = L"res";
  auto test_folder_dir = xe::fix_path_separators(L"src/xenia/base/testing");
  auto test_folder_path = xe::join_paths(test_folder_dir, test_folder_name);

  filesystem::FileInfo info = {};

  REQUIRE(filesystem::GetInfo(test_folder_path, &info));

  CHECK(info.type == filesystem::FileInfo::Type::kDirectory);
  CHECK(info.name == test_folder_name);
  CHECK(info.path == test_folder_dir);
  CHECK(info.total_size == 0);
  CHECK(info.create_timestamp > 132111406279379842);
  CHECK(info.access_timestamp > 132111406279379842);
  CHECK(info.write_timestamp > 132111406279379842);
}

}  // namespace test
}  // namespace base
}  // namespace xe
