cmake_minimum_required(VERSION 3.21)
project(Day36StructuredLogging LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(DAY36_ENABLE_SANITIZERS "Enable AddressSanitizer and UBSan" OFF)
option(DAY36_BUILD_EXERCISES "Build learner exercise tests" OFF)

add_library(audio_service_core
    src/error_code.cpp
    src/structured_logger.cpp
    src/inference_service.cpp
)
target_include_directories(audio_service_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(audio_service_core PUBLIC cxx_std_17)
set_target_properties(audio_service_core PROPERTIES CXX_EXTENSIONS OFF)
target_compile_options(audio_service_core PRIVATE
    "$<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wall;-Wextra;-Wpedantic>"
    "$<$<CXX_COMPILER_ID:MSVC>:/W4>"
)

if(DAY36_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "AppleClang|Clang|GNU")
    add_library(project_sanitizers INTERFACE)
    target_compile_options(project_sanitizers INTERFACE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
    target_link_options(project_sanitizers INTERFACE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
    target_link_libraries(audio_service_core PUBLIC project_sanitizers)
endif()

add_executable(audio_service_demo app/main.cpp)
target_link_libraries(audio_service_demo PRIVATE audio_service_core)

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

    add_executable(audio_service_tests
        tests/error_code_test.cpp
        tests/structured_logger_test.cpp
        tests/inference_service_test.cpp
    )
    target_link_libraries(audio_service_tests PRIVATE
        audio_service_core GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(audio_service_tests DISCOVERY_TIMEOUT 30)

    if(DAY36_BUILD_EXERCISES)
        add_executable(audio_service_exercise_tests
            exercises/01_error_mapping_and_service_test.cpp
            exercises/02_structured_log_test.cpp
        )
        target_link_libraries(audio_service_exercise_tests PRIVATE
            audio_service_core GTest::gtest_main
        )
        gtest_discover_tests(audio_service_exercise_tests DISCOVERY_TIMEOUT 30)
    endif()
endif()
