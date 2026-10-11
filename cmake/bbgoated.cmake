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
  target_sources(bbhost PRIVATE src/host/bbgoated.cpp)
endif()
