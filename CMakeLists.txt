cmake_minimum_required(VERSION 3.20)

if(WIN32)
    project(SimpleApp LANGUAGES CXX)
else()
    project(SimpleApp LANGUAGES CXX ASM)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

find_package(imgui CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# -----------------------------------------------------------------------------
# Modular CMake Modules
# -----------------------------------------------------------------------------
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/SetupFonts.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/SetupResources.cmake")

# -----------------------------------------------------------------------------
# Main Application Sources
# -----------------------------------------------------------------------------
set(SOURCE_FILES
    .clang-format
    src/main.cpp
    src/core/app.h
    src/core/app.cpp
    src/core/text_manager.h
    src/core/text_manager.cpp
    src/window/window.h
    src/window/window.cpp
    src/window/native_window.h
    src/graphics/grid_bg.h
    src/graphics/grid_bg.cpp
    src/ui/text_renderer.h
    src/ui/text_renderer.cpp
    src/ui/title_bar.h
    src/ui/title_bar.cpp
    src/ui/settings_modal.h
    src/ui/settings_modal.cpp
    src/ui/dashboard_cards.h
    src/ui/dashboard_cards.cpp
    src/system/system_metrics.h
    src/system/system_metrics.cpp
    src/resources/embedded_resources.h
    src/resources/embedded_resources.cpp
    src/utils/utf8_utils.h
    src/utils/utf8_utils.cpp
    src/utils/url_utils.h
    src/utils/url_utils.cpp
    ${EMBEDDED_RESOURCE_FILES}
)

if (WIN32)
    list(APPEND SOURCE_FILES src/window/window_win32.cpp)
    add_executable(SimpleApp WIN32 ${SOURCE_FILES})
    
    target_link_libraries(SimpleApp PRIVATE 
        imgui::imgui 
        d3d11 
        d3dcompiler 
        dxgi 
        dwmapi 
        uxtheme
    )
    target_compile_definitions(SimpleApp PRIVATE UNICODE _UNICODE IMGUI_USE_WCHAR32)
else()
    list(APPEND SOURCE_FILES src/window/window_glfw.cpp)
    add_executable(SimpleApp ${SOURCE_FILES})
    
    find_package(glfw3 CONFIG REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(X11)
    
    target_link_libraries(SimpleApp PRIVATE
        imgui::imgui
        glfw
        OpenGL::GL
    )
    if(X11_FOUND)
        target_link_libraries(SimpleApp PRIVATE ${X11_LIBRARIES})
        if(X11_Xext_LIB)
            target_link_libraries(SimpleApp PRIVATE ${X11_Xext_LIB})
        endif()
    endif()
    target_compile_definitions(SimpleApp PRIVATE IMGUI_USE_WCHAR32)
endif()

target_link_libraries(SimpleApp PRIVATE nlohmann_json::nlohmann_json)
target_include_directories(SimpleApp PRIVATE src "${GEN_DIR}")

install(TARGETS SimpleApp DESTINATION .)
