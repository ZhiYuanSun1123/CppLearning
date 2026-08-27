cmake_minimum_required(VERSION 3.20)

project(
    TensorMetadataConsumer
    LANGUAGES CXX
)

find_package(
    TensorMetadata 1.0
    CONFIG
    REQUIRED
)

add_executable(
    tensor_metadata_consumer
    main.cpp
)

target_compile_features(
    tensor_metadata_consumer
    PRIVATE
    cxx_std_17
)

set_target_properties(
    tensor_metadata_consumer
    PROPERTIES
    CXX_EXTENSIONS OFF
)

target_link_libraries(
    tensor_metadata_consumer
    PRIVATE
    TensorMetadata::tensor_metadata
)
