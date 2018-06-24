# build native_app_glue as a static lib
cmake_minimum_required(VERSION 3.4.1)

add_library(NdkHelper
  STATIC
    gl3stub.cpp
    GLContext.cpp

)
set_target_properties(NdkHelper
  PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    INTERFACE_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:NdkHelper,INCLUDE_DIRECTORIES>
)
target_include_directories(NdkHelper
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(NdkHelper
  PUBLIC
    common
    GLESv2
    EGL
    atomic
)
