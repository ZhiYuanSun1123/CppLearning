# 设置基础设置
cmake_minimum_required(VERSION 3.20)
project(
    TensorMetaDataProject
    VERSION 1.0
    LANGUAGES CXX
)

include(GNUInstallDirs)

# 构建包版本配置文件
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/TensorMetadataConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
# 设置属性
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.2.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
    CLI11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.5.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(CLI11)


# 构建一个INTERFACE库来保存构建参数
add_library(project_warnings INTERFACE)
target_compile_options(
    project_warnings
    INTERFACE
    "$<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wall;-Wextra;-Wpedantic>"
    "$<$<CXX_COMPILER_ID:MSVC>:/W4>"
)

# 创建一个静态库
add_library(tensor_metadata STATIC
    src/data_type.cpp
    src/device.cpp
    src/tensor_layout.cpp
    src/tensor_shape.cpp
    src/tensor_metadata.cpp
)
target_include_directories(
    tensor_metadata
    PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
## 设置这个库的c++标准
target_compile_features(
    tensor_metadata
    PUBLIC
    cxx_std_17
)
set_target_properties(
    tensor_metadata
    PROPERTIES
    CXX_EXTENSIONS OFF
)

# 把INTERFACE库的构建配置链接到tensor_metadata里面
target_link_libraries(
    tensor_metadata
    PRIVATE
    "$<BUILD_INTERFACE:project_warnings>" # 只要求在构建阶段保留一些编译信息
)
# 配置安装
install(
    TARGETS tensor_metadata
    EXPORT TensorMetadataTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING
    PATTERN "*.hpp"
)

# 创建可执行程序target
add_executable( tensor_metadata_demo
    app/main.cpp
)

set_target_properties(
    tensor_metadata_demo
    PROPERTIES
    CXX_EXTENSIONS OFF
)

target_link_libraries(
    tensor_metadata_demo
    PRIVATE
    tensor_metadata
    "$<BUILD_INTERFACE:project_warnings>"
    fmt::fmt
    CLI11::CLI11
)
target_compile_definitions(
    tensor_metadata_demo
    PRIVATE
    "$<$<CONFIG:Debug>:TENSOR_DEMO_DEBUG=1>"
    "$<$<CONFIG:Release>:TENSOR_DEMO_RELEASE=1>"
)

install(
    TARGETS tensor_metadata_demo
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    EXPORT TensorMetadataTargets
    FILE TensorMetadataTargets.cmake
    NAMESPACE TensorMetadata::
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/TensorMetadata
)
# 生成包配置文件
include(CMakePackageConfigHelpers)

set(
    TensorMetadata_INSTALL_CMAKEDIR
    ${CMAKE_INSTALL_LIBDIR}/cmake/TensorMetadata
)

configure_package_config_file(
    cmake/TensorMetadataConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/TensorMetadataConfig.cmake
    INSTALL_DESTINATION
        ${TensorMetadata_INSTALL_CMAKEDIR}
)

# 安装包配置和版本文件
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/TensorMetadataConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/TensorMetadataConfigVersion.cmake
    DESTINATION
        ${TensorMetadata_INSTALL_CMAKEDIR}
)


# 构建测试
include(CTest)
if(BUILD_TESTING)
    add_executable( tensor_metadata_tests
        tests/test_tensor_metadata.cpp
    )
    target_link_libraries(
        tensor_metadata_tests
        PRIVATE
        tensor_metadata
        project_warnings
    )
    set_target_properties(
        tensor_metadata_tests
        PROPERTIES
        CXX_EXTENSIONS OFF
    )
    add_test(
        NAME tensor_metadata_tests
        COMMAND tensor_metadata_tests
    )
endif()
