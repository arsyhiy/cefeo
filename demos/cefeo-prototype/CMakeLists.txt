cmake_minimum_required(VERSION 3.16)
project(cefeo)

# Ядро
add_library(cefeo_core
    core/core.c
)

target_include_directories(cefeo_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core)

# Windows оболочка
if (WIN32)
    add_executable(cefeo_win
        platform/windows/main.cpp
    )
    target_link_libraries(cefeo_win cefeo_core)
endif()

# Linux оболочка
if (UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    add_executable(cefeo_linux
        platform/linux/main.cpp
    )
    target_link_libraries(cefeo_linux cefeo_core X11::X11)
endif()
