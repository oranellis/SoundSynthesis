cmake_minimum_required(VERSION 3.13)

project(SoundSynthesis)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD 20)

add_executable(sounds)

add_subdirectory(src)

target_include_directories(sounds PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)

add_custom_target(run ALL ${CMAKE_SOURCE_DIR}/scripts/run.sh
    DEPENDS sounds)
