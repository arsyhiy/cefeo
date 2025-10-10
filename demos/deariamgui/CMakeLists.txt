cmake_minimum_required(VERSION 3.15)
project(ImGuiDX12Template LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FS")
endif()

# ImGui static lib
file(GLOB IMGUI_SRC
    lib/imgui/*.cpp
    lib/imgui/backends/imgui_impl_dx12.cpp
    lib/imgui/backends/imgui_impl_win32.cpp
)
add_library(imgui STATIC ${IMGUI_SRC})
target_include_directories(imgui PUBLIC lib/imgui lib/imgui/backends)
target_compile_definitions(imgui PUBLIC IMGUI_DISABLE_OBSOLETE_FUNCTIONS)

# Main app
file(GLOB SRC src/*.cpp)
add_executable(ImGuiDX12Template ${SRC})
target_include_directories(ImGuiDX12Template PRIVATE src lib/imgui lib/imgui/backends)
target_link_libraries(ImGuiDX12Template PRIVATE imgui d3d12 dxgi dxguid d3dcompiler)

if (MSVC)
    set_target_properties(ImGuiDX12Template PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    )
endif()
