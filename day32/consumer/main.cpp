#include "tensor_metadata.hpp"

#include <iostream>

int main() {
    const TensorMetadata metadata(
        "consumer_tensor",
        TensorShape({1, 200, 80}),
        DataType::float32,
        TensorLayout::batch_time_feature,
        Device(DeviceType::cpu, 0)
    );

    std::cout
        << metadata.name()
        << ": "
        << metadata.shape().rank()
        << '\n';

    return 0;
}
