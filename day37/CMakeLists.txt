cmake_minimum_required(VERSION 3.21)
project(Day37GitHubActions LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(audio_metadata src/audio_metadata.cpp)
target_include_directories(audio_metadata PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(audio_metadata PUBLIC cxx_std_17)
set_target_properties(audio_metadata PROPERTIES CXX_EXTENSIONS OFF)
target_compile_options(audio_metadata PRIVATE
    "$<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wall;-Wextra;-Wpedantic>"
    "$<$<CXX_COMPILER_ID:MSVC>:/W4>"
)

add_executable(audio_metadata_demo app/main.cpp)
target_link_libraries(audio_metadata_demo PRIVATE audio_metadata)

include(CTest)
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

    add_executable(audio_metadata_tests tests/audio_metadata_test.cpp)
    target_link_libraries(audio_metadata_tests PRIVATE
        audio_metadata GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(audio_metadata_tests DISCOVERY_TIMEOUT 30)
endif()
