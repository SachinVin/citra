add_subdirectory(ndk_helper)

add_library(main SHARED
    button_manager.cpp
    button_manager.h
    config.cpp
    config.h
    default_ini.h
    emu_window/emu_window.cpp
    emu_window/emu_window.h
    game_info.cpp
    game_info.h
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
  ndk_helper
  "-Wl,--no-warn-mismatch"
  "-Wl,--whole-archive"
  "-Wl,--no-whole-archive"
)

set(CPACK_PACKAGE_EXECUTABLES ${CPACK_PACKAGE_EXECUTABLES} main)
