project_root = "../../../.."
include(project_root.."/tools/build")

group("src")
project("xenia-ui-spirv")
  uuid("2323a069-5b29-44a3-b524-f35451a81978")
  kind("StaticLib")
  language("C++")
  links({
    "glslang-spirv",
    "SPIRV-Tools",
    "xenia-base",
  })
  defines({
  })
  includedirs({
    project_root.."/third_party/gflags/src",
  })
  local_platform_files()
