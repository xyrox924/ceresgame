cmake_minimum_required(VERSION 3.16)

project(ceres VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(ceres
    src/actor.c
    src/animation.c
    src/camera.c
    third_party/cjson/cJSON.c
    src/main.c
    src/map.c
    src/resources.c
    src/solid.c
    src/texture.c
    src/timer.c
)

target_include_directories(ceres PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/cjson"
)
target_compile_definitions(ceres PRIVATE
    $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
set_target_properties(ceres PROPERTIES
    WIN32_EXECUTABLE $<AND:$<PLATFORM_ID:Windows>,$<NOT:$<CONFIG:Debug>>>
)

macro(require_pkg_config)
    if(NOT PkgConfig_FOUND)
        find_package(PkgConfig QUIET)
    endif()

    if(NOT PkgConfig_FOUND)
        message(FATAL_ERROR
            "SDL dependencies were not found as CMake packages, and pkg-config "
            "is unavailable. Install SDL2, SDL2_image, and SDL2_mixer with your "
            "system package manager, or configure with the vcpkg toolchain file."
        )
    endif()
endmacro()

find_package(SDL2 CONFIG QUIET COMPONENTS SDL2)
if(TARGET SDL2::SDL2)
    find_package(SDL2 CONFIG QUIET COMPONENTS SDL2main)
else()
    require_pkg_config()
    pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
endif()

find_package(SDL2_image CONFIG QUIET)
if(NOT TARGET SDL2_image::SDL2_image)
    require_pkg_config()
    pkg_check_modules(SDL2_IMAGE REQUIRED IMPORTED_TARGET SDL2_image)
endif()

find_package(SDL2_mixer CONFIG QUIET)
if(NOT TARGET SDL2_mixer::SDL2_mixer)
    require_pkg_config()
    pkg_check_modules(SDL2_MIXER REQUIRED IMPORTED_TARGET SDL2_mixer)
endif()

if(TARGET SDL2::SDL2main)
    target_link_libraries(ceres PRIVATE SDL2::SDL2main)
endif()

if(TARGET SDL2::SDL2)
    target_link_libraries(ceres PRIVATE SDL2::SDL2)
else()
    target_link_libraries(ceres PRIVATE PkgConfig::SDL2)
endif()

if(TARGET SDL2_image::SDL2_image)
    target_link_libraries(ceres PRIVATE SDL2_image::SDL2_image)
else()
    target_link_libraries(ceres PRIVATE PkgConfig::SDL2_IMAGE)
endif()

if(TARGET SDL2_mixer::SDL2_mixer)
    target_link_libraries(ceres PRIVATE SDL2_mixer::SDL2_mixer)
else()
    target_link_libraries(ceres PRIVATE PkgConfig::SDL2_MIXER)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(ceres PRIVATE m)
endif()

add_custom_command(TARGET ceres POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/res"
            "$<TARGET_FILE_DIR:ceres>/res"
    VERBATIM
)

install(TARGETS ceres RUNTIME DESTINATION .)
install(DIRECTORY res DESTINATION .)
