cmake_minimum_required(VERSION 3.13)

project(SoundSynthesis)

find_package(ALSA REQUIRED)
include_directories(${ALSA_INCLUDE_DIRS})

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD 20)

add_executable(sounds)

add_subdirectory(src)

target_include_directories(sounds PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(sounds ${ALSA_LIBRARIES})

add_custom_target(run ${CMAKE_SOURCE_DIR}/scripts/run.sh
    DEPENDS sounds)
