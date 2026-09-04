cmake_minimum_required(VERSION 3.21)
project(Day38TestReports LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(audio_task_queue src/audio_task_queue.cpp)
target_include_directories(audio_task_queue PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(audio_task_queue PUBLIC cxx_std_17)
set_target_properties(audio_task_queue PROPERTIES CXX_EXTENSIONS OFF)
target_compile_options(audio_task_queue PRIVATE
    "$<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wall;-Wextra;-Wpedantic>"
    "$<$<CXX_COMPILER_ID:MSVC>:/W4>"
)

add_executable(audio_task_queue_demo app/main.cpp)
target_link_libraries(audio_task_queue_demo PRIVATE audio_task_queue)

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

    add_executable(audio_task_queue_tests tests/audio_task_queue_test.cpp)
    target_link_libraries(audio_task_queue_tests PRIVATE
        audio_task_queue GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(audio_task_queue_tests DISCOVERY_TIMEOUT 30)
endif()
