# bbgoated (docs/bbgoated/UPSTREAM.md): everything bbgoated adds to the build,
# in one file included at the end of CMakeLists.txt, so an upstream rebase
# touches one line there.
option(BBGOATED "bbgoated: share bbhost's per-user config, keep a data folder of its own" OFF)

if(BBGOATED)
  target_compile_definitions(bbhost PRIVATE BBHOST_BBGOATED=1)
  message(STATUS "bbhost: bbgoated build (config shared with bbhost, own data folder)")
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
  target_sources(bbhost PRIVATE src/host/bbgoated.cpp src/host/redzone_audit.cpp src/host/options_carry.cpp)
endif()

# The patch manifests' cpu_vendor gate (engine/patch_cpu.*), tested on its own.
if(TARGET config_test)
  add_executable(patch_manifest_cpu_test tests/patch_manifest_cpu_test.cpp src/engine/patch_cpu.cpp)
  target_include_directories(patch_manifest_cpu_test PRIVATE src)
  add_test(NAME patch_manifest_cpu_test COMMAND patch_manifest_cpu_test)
endif()
# Every build: the manifests read cpu_vendor (engine/patch_manifest.cpp).
target_sources(bbhost PRIVATE src/engine/patch_cpu.cpp)

# The red-zone audit's site table (host/redzone_audit.*), tested on its own.
if(TARGET config_test AND BBGOATED)
  add_executable(redzone_audit_test tests/redzone_audit_test.cpp src/host/redzone_audit.cpp)
  target_include_directories(redzone_audit_test PRIVATE src)
  target_compile_definitions(redzone_audit_test PRIVATE BBHOST_REDZONE_AUDIT_TABLE_ONLY=1)
  add_test(NAME redzone_audit_test COMMAND redzone_audit_test)
endif()

# The shared options file keeps keys bbgoated does not know (host/options_carry.*).
if(TARGET config_test AND BBGOATED)
  add_executable(options_carry_test tests/options_carry_test.cpp src/host/options_carry.cpp)
  target_include_directories(options_carry_test PRIVATE src)
  add_test(NAME options_carry_test COMMAND options_carry_test)
endif()
