cmake_minimum_required(VERSION 3.21)
project(Day34GoogleTest LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 1. 被测试的业务库。
add_library(audio_core src/audio_clip.cpp)
target_include_directories(audio_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(audio_core PUBLIC cxx_std_17)
set_target_properties(audio_core PROPERTIES CXX_EXTENSIONS OFF)
target_compile_options(audio_core PRIVATE
    "$<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wall;-Wextra;-Wpedantic>"
    "$<$<CXX_COMPILER_ID:MSVC>:/W4>"
)

# 2. 普通应用，拥有自己的 main。
add_executable(audio_demo app/main.cpp)
target_link_libraries(audio_demo PRIVATE audio_core)

# 3. CTest 和测试开关。
include(CTest)
option(DAY34_BUILD_EXERCISES "Build learner exercise tests" OFF)

if(BUILD_TESTING)
    # 4. 下载并加入固定版本的 GoogleTest。
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

    # 5. 测试应用；main 来自 GTest::gtest_main。
    add_executable(audio_core_tests tests/audio_clip_test.cpp)
    target_link_libraries(audio_core_tests PRIVATE audio_core GTest::gtest_main)
    set_target_properties(audio_core_tests PROPERTIES CXX_EXTENSIONS OFF)

    # 6. 将每个 TEST 注册到 CTest。
    include(GoogleTest)
    gtest_discover_tests(audio_core_tests DISCOVERY_TIMEOUT 30)

    # 练习单独运行，默认不把未完成的题算进参考测试。
    if(DAY34_BUILD_EXERCISES)
        add_executable(audio_exercise_tests
            exercises/01_metadata_normal_test.cpp
            exercises/02_batch_normal_test.cpp
        )
        target_link_libraries(audio_exercise_tests PRIVATE audio_core GTest::gtest_main)
        gtest_discover_tests(audio_exercise_tests DISCOVERY_TIMEOUT 30)
    endif()
endif()
