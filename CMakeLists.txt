cmake_minimum_required(VERSION 3.21)

# ── Project ──────────────────────────────────────────────────────────────────
project(
    SheathedShoutsAimAtCrosshair
    VERSION 1.0.0
    DESCRIPTION "Sheathed shouts aim toward the camera/crosshair direction"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Dependencies ─────────────────────────────────────────────────────────────
find_package(CommonLibSSE REQUIRED)
find_package(SimpleIni CONFIG REQUIRED)

# ── Sources ──────────────────────────────────────────────────────────────────
file(GLOB_RECURSE SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE HEADERS
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

# ── Plugin target ────────────────────────────────────────────────────────────
# add_commonlibsse_plugin handles:
#   - Creating a shared library (.dll)
#   - Injecting SKSE plugin version metadata
#   - Setting up the correct export symbols
add_commonlibsse_plugin(
    ${PROJECT_NAME}
    SOURCES ${SOURCES} ${HEADERS}
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        CommonLibSSE::CommonLibSSE
        SimpleIni::SimpleIni
)

target_precompile_headers(
    ${PROJECT_NAME}
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include/PCH.h"
)

# ── Compiler options ─────────────────────────────────────────────────────────
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4         # Warning level 4
        /permissive-
        /Zc:preprocessor
        /external:anglebrackets
        /external:W0
    )
endif()

# ── Post-build: copy DLL + INI to dist/ ─────────────────────────────────────
set(DIST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dist/SKSE/Plugins")

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:${PROJECT_NAME}>"
        "${DIST_DIR}/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/Data/SKSE/Plugins/SheathedShoutsAimAtCrosshair.ini"
        "${DIST_DIR}/"
    COMMENT "Copying plugin DLL and INI to dist/"
)

# ── Optional: auto-deploy to Skyrim mods folder ─────────────────────────────
if(DEFINED ENV{SKYRIM_MODS_FOLDER})
    set(MOD_DIR "$ENV{SKYRIM_MODS_FOLDER}/SheathedShoutsAimAtCrosshair/SKSE/Plugins")
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${MOD_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${PROJECT_NAME}>"
            "${MOD_DIR}/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/Data/SKSE/Plugins/SheathedShoutsAimAtCrosshair.ini"
            "${MOD_DIR}/"
        COMMENT "Deploying plugin to Skyrim mods folder"
    )
endif()
