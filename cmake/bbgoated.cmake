# bbgoated (docs/bbgoated/UPSTREAM.md): everything bbgoated adds to the build,
# in one file included at the end of CMakeLists.txt, so an upstream rebase
# touches one line there.
option(BBGOATED "bbgoated: a config and data folder of its own, filled once from bbhost's" OFF)

if(BBGOATED)
  target_compile_definitions(bbhost PRIVATE BBHOST_BBGOATED=1)
  message(STATUS "bbhost: bbgoated build (its own config and data folders)")
endif()

# Tests of bbgoated's own code, beside upstream's (enable_testing is UNIX-only).
if(TARGET config_test AND BBGOATED)
  add_executable(bbgoated_config_test tests/bbgoated_config_test.cpp src/core/config.cpp src/core/sfo.cpp)
  target_include_directories(bbgoated_config_test PRIVATE src ${CMAKE_BINARY_DIR}/gen)
  target_compile_definitions(bbgoated_config_test PRIVATE BBHOST_BBGOATED=1)
  add_dependencies(bbgoated_config_test bbhost_version)
  add_test(NAME bbgoated_config_test COMMAND bbgoated_config_test)
endif()

# bbgoated's host-side additions (src/host/bbgoated.*): the other-instance warning.
if(BBGOATED)
  target_sources(bbhost PRIVATE src/host/bbgoated.cpp src/host/redzone_audit.cpp)
endif()

# The patch manifests' cpu_vendor gate (engine/patch_cpu.*), tested on its own.
if(TARGET config_test)
  add_executable(patch_manifest_cpu_test tests/patch_manifest_cpu_test.cpp src/engine/patch_cpu.cpp)
  target_include_directories(patch_manifest_cpu_test PRIVATE src)
  add_test(NAME patch_manifest_cpu_test COMMAND patch_manifest_cpu_test)
endif()
# Every build: the manifests read cpu_vendor (engine/patch_manifest.cpp).
target_sources(bbhost PRIVATE src/engine/patch_cpu.cpp)

# A vertex range staged once a recording serves its later draws (host/stage_share.h).
if(TARGET config_test)
  add_executable(stage_share_test tests/stage_share_test.cpp)
  target_include_directories(stage_share_test PRIVATE src)
  add_test(NAME stage_share_test COMMAND stage_share_test)
endif()

# bbgoated performance switches (docs/bbgoated/SWITCHES.md): every source in
# src/host/perf - its renderer glue (perf/vk) only with Vulkan - so a switch's
# task adds files without editing this one. Their pure parts are header-only
# (src/host/perf/core/*.h), each with a test: tests/bbg_<name>_test.cpp.
file(GLOB BBG_PERF_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/host/perf/*.cpp)
target_sources(bbhost PRIVATE ${BBG_PERF_SOURCES})
if(Vulkan_FOUND)
  file(GLOB BBG_PERF_VK_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/host/perf/vk/*.cpp)
  target_sources(bbhost PRIVATE ${BBG_PERF_VK_SOURCES})
endif()
if(TARGET config_test)
  file(GLOB BBG_PERF_TESTS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/tests/bbg_*_test.cpp)
  foreach(t ${BBG_PERF_TESTS})
    get_filename_component(n ${t} NAME_WE)
    add_executable(${n} ${t})
    target_include_directories(${n} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    add_test(NAME ${n} COMMAND ${n})
  endforeach()
endif()

# The red-zone audit's site table (host/redzone_audit.*), tested on its own.
if(TARGET config_test AND BBGOATED)
  add_executable(redzone_audit_test tests/redzone_audit_test.cpp src/host/redzone_audit.cpp)
  target_include_directories(redzone_audit_test PRIVATE src)
  target_compile_definitions(redzone_audit_test PRIVATE BBHOST_REDZONE_AUDIT_TABLE_ONLY=1)
  add_test(NAME redzone_audit_test COMMAND redzone_audit_test)
endif()

# DLAA (the YoucefNabil/bbhost fork, src/host/dlaa.*): NVIDIA DLAA through the runtime-loaded
# ngx_bridge.dll, wherever the Vulkan renderer is built.
if(Vulkan_FOUND)
  target_sources(bbhost PRIVATE src/host/dlaa.cpp)
  set_source_files_properties(src/host/dlaa.cpp PROPERTIES COMPILE_OPTIONS "-Wno-missing-field-initializers")
endif()

# DLAA and the game's own anti-aliasing (host/dlaa_aa.*), tested on its own.
if(Vulkan_FOUND)
  target_sources(bbhost PRIVATE src/host/dlaa_aa.cpp)
endif()
if(TARGET config_test)
  add_executable(dlaa_aa_test tests/dlaa_aa_test.cpp src/host/dlaa_aa.cpp)
  target_include_directories(dlaa_aa_test PRIVATE src)
  add_test(NAME dlaa_aa_test COMMAND dlaa_aa_test)
endif()
