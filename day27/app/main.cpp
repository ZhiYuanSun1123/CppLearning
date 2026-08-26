#include "tensor_metadata.hpp"
#include<fmt/format.h>
#include <cstddef>
#include <exception>
#include <iostream>

namespace {

void print_shape(const TensorShape& shape) {
    std::cout << '[';

    const auto& dimensions = shape.dimensions();
    for (std::size_t index = 0;
         index < dimensions.size();
         ++index) {
        if (index != 0) {
            std::cout << ',';
        }
        std::cout << dimensions[index];
    }

    std::cout << ']';
}

void print_metadata(const TensorMetadata& metadata) {
    std::cout << "name=" << metadata.name() << '\n';

    std::cout << "shape=";
    print_shape(metadata.shape());
    std::cout << '\n';

    std::cout
        << "rank=" << metadata.shape().rank() << '\n'
        << "dtype=" << to_string(metadata.data_type()) << '\n'
        << "layout=" << to_string(metadata.layout()) << '\n'
        << "device=" << to_string(metadata.device()) << '\n';

    const auto element_count = metadata.shape().numel();
    if (element_count.has_value()) {
        std::cout << "numel=" << *element_count << '\n';
    } else {
        std::cout << "numel=overflow\n";
    }

    const auto bytes = metadata.byte_size();
    if (bytes.has_value()) {
        std::cout << "bytes=" << *bytes << '\n';
    } else {
        std::cout << "bytes=overflow\n";
    }
}

} // namespace
[[nodiscard]] const char* build_mode() noexcept {
#if defined(TENSOR_DEMO_DEBUG)
    return "Debug";
#elif defined(TENSOR_DEMO_RELEASE)
    return "Release";
#else
    return "Unknown";
#endif
}

int main() {
    try {
        const TensorMetadata metadata(
            "mel_features",
            TensorShape({1, 3000, 80}),
            DataType::float32,
            TensorLayout::batch_time_feature,
            Device(DeviceType::cpu, 0)
        );

        print_metadata(metadata);
    } catch (const std::exception& error) {
        std::cerr
            << "创建张量元数据失败: "
            << error.what()
            << '\n';
        return 1;
    }

    std::cout
        << fmt::format(
            "Tensor metadata demo [{}]",
            build_mode()
        )
        << std::endl;

    return 0;
}
