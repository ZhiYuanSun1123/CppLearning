#include "tensor_metadata.hpp"
#include<fmt/format.h>
#include <cstddef>
#include <exception>
#include <iostream>
#include<CLI/CLI.hpp>

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

int main(int argc,char* argv[]) {
    CLI::App app{
        "Inspect tensor metadata for an audio inference input"
    };
    std::string name = "mel_features";
    std::size_t batch = 1;
    std::size_t time = 3000;
    std::size_t feature = 80;
    int device_index = 0;

    app.add_option(
        "--name",
        name,
        "Tensor name"
    );

    app.add_option(
        "--batch",
        batch,
        "Batch dimension"
    )->check(CLI::PositiveNumber);

    app.add_option(
        "--time",
        time,
        "Time-frame dimension"
    )->check(CLI::PositiveNumber);

    app.add_option(
        "--feature",
        feature,
        "Feature dimension"
    )->check(CLI::PositiveNumber);

    app.add_option(
        "--device-index",
        device_index,
        "Accelerator device index"
    )->check(CLI::NonNegativeNumber);
    CLI11_PARSE(app, argc, argv);
    try {
        const TensorMetadata metadata(
            "mel_features",
            TensorShape({batch, time, feature}),
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
