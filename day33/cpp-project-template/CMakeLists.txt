cmake_minimum_required(VERSION 3.21)

project(
    AudioCppTemplate
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(project_warnings INTERFACE)

target_compile_options(
    project_warnings
    INTERFACE
    "$<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wall;-Wextra;-Wpedantic>"
    "$<$<CXX_COMPILER_ID:MSVC>:/W4>"
)

add_library(
    audio_core
    src/audio_clip.cpp
)

add_library(
    AudioCppTemplate::audio_core
    ALIAS
    audio_core
)

target_include_directories(
    audio_core
    PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)

target_compile_features(
    audio_core
    PUBLIC
    cxx_std_17
)

set_target_properties(
    audio_core
    PROPERTIES
    CXX_EXTENSIONS OFF
)

target_link_libraries(
    audio_core
    PRIVATE
    project_warnings
)

add_executable(
    audio_demo
    app/main.cpp
)

target_link_libraries(
    audio_demo
    PRIVATE
    AudioCppTemplate::audio_core
    project_warnings
)

include(CTest)

if(BUILD_TESTING)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
        GIT_SHALLOW TRUE
    )

    FetchContent_MakeAvailable(googletest)

    add_executable(
        audio_core_tests
        tests/audio_clip_test.cpp
    )

    target_link_libraries(
        audio_core_tests
        PRIVATE
        AudioCppTemplate::audio_core
        GTest::gtest_main
        project_warnings
    )

    include(GoogleTest)
    gtest_discover_tests(audio_core_tests)
endif()

