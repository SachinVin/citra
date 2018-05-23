add_subdirectory(ndk_helper)

add_library(main SHARED
    #ButtonManager.cpp
    #MainAndroid.cpp
    config.cpp
    config.h
    emu_window/emu_window.cpp
    emu_window/emu_window.h
    native.cpp
    native.h
)

target_link_libraries(main
PRIVATE
    common
    core
    input_common
    network
    glad
    inih
)

target_link_libraries(main
PRIVATE
  android
  log
  NdkHelper
  "-Wl,--no-warn-mismatch"
  "-Wl,--whole-archive"
  "-Wl,--no-whole-archive"
)

set(CPACK_PACKAGE_EXECUTABLES ${CPACK_PACKAGE_EXECUTABLES} main)
