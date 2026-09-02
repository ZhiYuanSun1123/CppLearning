cmake_minimum_required(VERSION 3.21)
project(Day35FailureAndResourceTests LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(DAY35_ENABLE_SANITIZERS "Enable AddressSanitizer and UBSan" OFF)

add_library(audio_core
    src/audio_clip.cpp
    src/audio_session.cpp
)
target_include_directories(audio_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(audio_core PUBLIC cxx_std_17)
set_target_properties(audio_core PROPERTIES CXX_EXTENSIONS OFF)
target_compile_options(audio_core PRIVATE
    "$<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wall;-Wextra;-Wpedantic>"
    "$<$<CXX_COMPILER_ID:MSVC>:/W4>"
)

if(DAY35_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "AppleClang|Clang|GNU")
    add_library(project_sanitizers INTERFACE)
    target_compile_options(project_sanitizers INTERFACE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
    target_link_options(project_sanitizers INTERFACE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
    target_link_libraries(audio_core PUBLIC project_sanitizers)
endif()

add_executable(audio_demo app/main.cpp)
target_link_libraries(audio_demo PRIVATE audio_core)

include(CTest)
option(DAY35_BUILD_EXERCISES "Build learner exercise tests" OFF)

if(BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
        GIT_SHALLOW TRUE
    )
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    if(MSVC)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()
    FetchContent_MakeAvailable(googletest)

    add_executable(audio_core_tests
        tests/audio_clip_edge_test.cpp
        tests/audio_session_resource_test.cpp
    )
    target_link_libraries(audio_core_tests PRIVATE audio_core GTest::gtest_main)
    target_compile_features(audio_core_tests PRIVATE cxx_std_17)

    include(GoogleTest)
    gtest_discover_tests(audio_core_tests DISCOVERY_TIMEOUT 30)

    if(DAY35_BUILD_EXERCISES)
        add_executable(audio_exercise_tests
            exercises/01_audio_clip_edge_and_exception_test.cpp
            exercises/02_audio_session_release_test.cpp
        )
        target_link_libraries(audio_exercise_tests PRIVATE audio_core GTest::gtest_main)
        gtest_discover_tests(audio_exercise_tests DISCOVERY_TIMEOUT 30)
    endif()
endif()
