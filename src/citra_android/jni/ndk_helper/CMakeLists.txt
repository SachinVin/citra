# build native_app_glue as a static lib
cmake_minimum_required(VERSION 3.4.1)

add_library(ndk_helper
  STATIC
    gl3stub.cpp
    GLContext.cpp

)
set_target_properties(ndk_helper
  PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    INTERFACE_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:ndk_helper,INCLUDE_DIRECTORIES>
)
target_include_directories(ndk_helper
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ndk_helper
  PUBLIC
    common
    GLESv2
    EGL
    atomic
)
