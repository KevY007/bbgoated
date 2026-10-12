# bbgoated (docs/bbgoated/pr73-port.md): PR #73's upscaler sources and tests that bbgoated
# takes, and bbgoated's own upscaler files. Each block steps aside when upstream's
# CMakeLists.txt already registers the source or target (a later upstream merge of PR #73),
# so that merge only deletes lines here. CMakeLists.txt itself is not touched.
if(Vulkan_FOUND)
  get_target_property(_bbg_sources bbhost SOURCES)
  foreach(_src src/host/dlaa.cpp src/host/object_motion.cpp)
    if(NOT _src IN_LIST _bbg_sources)
      target_sources(bbhost PRIVATE ${_src})
      set_source_files_properties(${_src} PROPERTIES COMPILE_OPTIONS "-Wno-missing-field-initializers")
    endif()
  endforeach()
  # The FSR provider arrives in Task 6; until then, and with -DBBGOATED_FSR=OFF, a stub.
  target_sources(bbhost PRIVATE src/host/fsr_upscaler_stub.cpp)
endif()

# Unit tests, in Linux test builds beside upstream's (its enable_testing() is UNIX-only).
if(TARGET config_test)
  if(NOT TARGET display_settings_test)
    add_executable(display_settings_test tests/display_settings_test.cpp)
    target_include_directories(display_settings_test PRIVATE src)
    target_compile_options(display_settings_test PRIVATE -UNDEBUG)
    add_test(NAME display_settings_test COMMAND display_settings_test)
  endif()
  if(Vulkan_FOUND AND NOT TARGET dlaa_motion_test)
    add_executable(dlaa_motion_test tests/dlaa_motion_test.cpp)
    target_include_directories(dlaa_motion_test PRIVATE src)
    target_link_libraries(dlaa_motion_test PRIVATE Vulkan::Vulkan)
    add_test(NAME dlaa_motion_test COMMAND dlaa_motion_test)
    set_tests_properties(dlaa_motion_test PROPERTIES SKIP_RETURN_CODE 77)
  endif()
  # The NGX bridge's barrier repair (tools/ngx_bridge, taken with the bridge), against a mock: no GPU.
  if(Vulkan_FOUND AND NOT TARGET ngx_transfer_sync_test)
    add_executable(ngx_transfer_sync_test tools/ngx_bridge/ngx_transfer_sync_test.cpp)
    find_package(Threads REQUIRED)
    target_link_libraries(ngx_transfer_sync_test PRIVATE Vulkan::Vulkan Threads::Threads)
    add_test(NAME ngx_transfer_sync_test COMMAND ngx_transfer_sync_test)
  endif()
endif()
