# -----------------------------------------------------------------------------
# Font Downloads & Asset Setup
# -----------------------------------------------------------------------------
set(FONT_DIR "${CMAKE_BINARY_DIR}/fonts")
file(MAKE_DIRECTORY "${FONT_DIR}")

set(JETBRAINS_MONO_TTF "${FONT_DIR}/JetBrainsMono-Regular.ttf")

if(EXISTS "${JETBRAINS_MONO_TTF}")
    file(SIZE "${JETBRAINS_MONO_TTF}" JETBRAINS_MONO_SIZE)
    if(JETBRAINS_MONO_SIZE LESS 1000)
        file(REMOVE "${JETBRAINS_MONO_TTF}")
    endif()
endif()

if(NOT EXISTS "${JETBRAINS_MONO_TTF}")
    message(STATUS "Downloading JetBrains Mono Regular...")
    file(DOWNLOAD "https://raw.githubusercontent.com/JetBrains/JetBrainsMono/master/fonts/ttf/JetBrainsMono-Regular.ttf" "${JETBRAINS_MONO_TTF}" SHOW_PROGRESS TLS_VERIFY ON)
endif()
