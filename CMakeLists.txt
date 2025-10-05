cmake_minimum_required(VERSION 3.16)
project(cefeo)

# ======================================
# Настройка директорий вывода (чисто)
# ======================================
# Все исполняемые файлы
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# Все статические и динамические библиотеки
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ======================================
# Ядро проекта (core)
# ======================================
add_library(cefeo_core core/core.c)
target_include_directories(cefeo_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core)

# Windows оболочка
if (WIN32)
    add_executable(cefeo_win WIN32 platform/windows/main.cpp)
    target_link_libraries(cefeo_win PRIVATE cefeo_core imgui d3d11 dxgi)
    target_include_directories(cefeo_win PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/core)
endif()

# ======================================
# Linux оболочка
# ======================================
if (UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    add_executable(cefeo_linux platform/linux/main.cpp)
    target_link_libraries(cefeo_linux PRIVATE cefeo_core X11::X11)
endif()

# ======================================
# Добавление исходников Dear ImGui
# ======================================
add_library(imgui
    lib/imgui/imgui.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/backends/imgui_impl_win32.cpp
    lib/imgui/backends/imgui_impl_dx11.cpp
)

# Указываем директории include для ImGui
target_include_directories(imgui PUBLIC
    ${CMAKE_SOURCE_DIR}/lib/imgui
    ${CMAKE_SOURCE_DIR}/lib/imgui/backends
)

