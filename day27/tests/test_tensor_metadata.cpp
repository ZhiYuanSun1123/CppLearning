#include "tensor_metadata.hpp"

#include <cstddef>
#include <functional>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string_view>

namespace {

class TestRunner {
public:
    void check(bool condition, std::string_view name) {
        ++total_;

        if (condition) {
            std::cout << "[PASS] " << name << '\n';
            return;
        }

        ++failed_;
        std::cerr << "[FAIL] " << name << '\n';
    }

    void expect_invalid_argument(
        const std::function<void()>& operation,
        std::string_view name
    ) {
        try {
            operation();
            check(false, name);
        } catch (const std::invalid_argument&) {
            check(true, name);
        } catch (...) {
            check(false, name);
        }
    }

    void expect_out_of_range(
        const std::function<void()>& operation,
        std::string_view name
    ) {
        try {
            operation();
            check(false, name);
        } catch (const std::out_of_range&) {
            check(true, name);
        } catch (...) {
            check(false, name);
        }
    }

    [[nodiscard]] int finish() const {
        std::cout
            << "\nTotal: " << total_
            << ", Passed: " << total_ - failed_
            << ", Failed: " << failed_
            << '\n';

        return failed_ == 0 ? 0 : 1;
    }

private:
    int total_ = 0;
    int failed_ = 0;
};

bool optional_equals(
    const std::optional<std::size_t>& value,
    std::size_t expected
) {
    return value.has_value() && *value == expected;
}

void test_data_types(TestRunner& runner) {
    runner.check(
        to_string(DataType::float32) == "float32",
        "float32名称"
    );
    runner.check(
        to_string(DataType::float16) == "float16",
        "float16名称"
    );
    runner.check(
        to_string(DataType::int32) == "int32",
        "int32名称"
    );
    runner.check(
        to_string(DataType::int8) == "int8",
        "int8名称"
    );
    runner.check(
        to_string(DataType::boolean) == "boolean",
        "boolean名称"
    );

    runner.check(
        optional_equals(bytes_per_element(DataType::float32), 4),
        "float32占4字节"
    );
    runner.check(
        optional_equals(bytes_per_element(DataType::float16), 2),
        "float16占2字节"
    );
    runner.check(
        optional_equals(bytes_per_element(DataType::int32), 4),
        "int32占4字节"
    );
    runner.check(
        optional_equals(bytes_per_element(DataType::int8), 1),
        "int8占1字节"
    );
    runner.check(
        optional_equals(bytes_per_element(DataType::boolean), 1),
        "boolean占1字节"
    );

    const auto invalid = static_cast<DataType>(100);
    runner.check(
        to_string(invalid) == "unknown",
        "非法dtype名称为unknown"
    );
    runner.check(
        !bytes_per_element(invalid).has_value(),
        "非法dtype没有字节数"
    );
}

void test_layouts(TestRunner& runner) {
    runner.check(
        to_string(TensorLayout::contiguous) == "contiguous",
        "contiguous名称"
    );
    runner.check(
        to_string(TensorLayout::batch_channel_time) ==
            "batch_channel_time",
        "batch_channel_time名称"
    );
    runner.check(
        to_string(TensorLayout::batch_time_feature) ==
            "batch_time_feature",
        "batch_time_feature名称"
    );
    runner.check(
        !expected_rank(TensorLayout::contiguous).has_value(),
        "contiguous不限制rank"
    );
    runner.check(
        optional_equals(
            expected_rank(TensorLayout::batch_channel_time),
            3
        ),
        "batch_channel_time要求rank 3"
    );
    runner.check(
        optional_equals(
            expected_rank(TensorLayout::batch_time_feature),
            3
        ),
        "batch_time_feature要求rank 3"
    );
}

void test_devices(TestRunner& runner) {
    const Device cpu(DeviceType::cpu, 0);
    const Device accelerator(DeviceType::accelerator, 2);

    runner.check(cpu.type() == DeviceType::cpu, "CPU类型");
    runner.check(cpu.index() == 0, "CPU编号");
    runner.check(to_string(cpu) == "cpu:0", "CPU字符串");
    runner.check(
        to_string(accelerator) == "accelerator:2",
        "加速器字符串"
    );
    runner.check(cpu == Device(DeviceType::cpu, 0), "设备相等");
    runner.check(!(cpu == accelerator), "设备不等");

    runner.expect_invalid_argument(
        [] {
            const Device invalid(DeviceType::cpu, 1);
            static_cast<void>(invalid);
        },
        "CPU拒绝非0编号"
    );
    runner.expect_invalid_argument(
        [] {
            const Device invalid(DeviceType::accelerator, -1);
            static_cast<void>(invalid);
        },
        "设备拒绝负编号"
    );
    runner.expect_invalid_argument(
        [] {
            const Device invalid(
                static_cast<DeviceType>(100),
                0
            );
            static_cast<void>(invalid);
        },
        "拒绝非法设备类型"
    );
}

void test_shapes(TestRunner& runner) {
    const TensorShape scalar;
    runner.check(scalar.rank() == 0, "标量rank为0");
    runner.check(scalar.is_scalar(), "默认shape是标量");
    runner.check(!scalar.is_empty_tensor(), "标量不是空张量");
    runner.check(optional_equals(scalar.numel(), 1), "标量有1个元素");

    const TensorShape normal({2, 3, 4});
    runner.check(normal.rank() == 3, "普通shape rank为3");
    runner.check(normal.at(0) == 2, "读取axis 0");
    runner.check(normal.at(1) == 3, "读取axis 1");
    runner.check(normal.at(2) == 4, "读取axis 2");
    runner.check(optional_equals(normal.numel(), 24), "普通shape numel");
    runner.check(!normal.is_scalar(), "普通shape不是标量");
    runner.check(!normal.is_empty_tensor(), "普通shape不是空张量");

    runner.expect_out_of_range(
        [&normal] {
            static_cast<void>(normal.at(3));
        },
        "越界axis抛出out_of_range"
    );

    const TensorShape empty({2, 0, 80});
    runner.check(empty.rank() == 3, "空张量rank仍为3");
    runner.check(empty.is_empty_tensor(), "零维度表示空张量");
    runner.check(optional_equals(empty.numel(), 0), "空张量numel为0");

    const std::size_t maximum =
        std::numeric_limits<std::size_t>::max();
    const TensorShape overflow({maximum, 2});
    runner.check(
        !overflow.numel().has_value(),
        "元素数溢出返回nullopt"
    );

    const TensorShape zero_after_huge({maximum, 2, 0});
    runner.check(
        optional_equals(zero_after_huge.numel(), 0),
        "包含零维度时元素数为0"
    );

    runner.check(
        TensorShape({2, 3}) == TensorShape({2, 3}),
        "相同shape相等"
    );
    runner.check(
        !(TensorShape({2, 3}) == TensorShape({3, 2})),
        "不同shape不等"
    );
}

void test_metadata(TestRunner& runner) {
    const TensorMetadata metadata(
        "mel_features",
        TensorShape({1, 3000, 80}),
        DataType::float32,
        TensorLayout::batch_time_feature,
        Device(DeviceType::cpu, 0)
    );

    runner.check(metadata.name() == "mel_features", "元数据名称");
    runner.check(metadata.shape().rank() == 3, "元数据rank");
    runner.check(
        metadata.data_type() == DataType::float32,
        "元数据dtype"
    );
    runner.check(
        metadata.layout() == TensorLayout::batch_time_feature,
        "元数据layout"
    );
    runner.check(
        metadata.device() == Device(DeviceType::cpu, 0),
        "元数据device"
    );
    runner.check(
        optional_equals(metadata.shape().numel(), 240000),
        "Mel特征元素数"
    );
    runner.check(
        optional_equals(metadata.byte_size(), 960000),
        "Mel特征字节数"
    );

    const TensorMetadata scalar(
        "score",
        TensorShape(),
        DataType::float32,
        TensorLayout::contiguous,
        Device(DeviceType::cpu, 0)
    );
    runner.check(
        optional_equals(scalar.byte_size(), 4),
        "float32标量占4字节"
    );

    const TensorMetadata empty(
        "empty_features",
        TensorShape({2, 0, 80}),
        DataType::float32,
        TensorLayout::batch_time_feature,
        Device(DeviceType::cpu, 0)
    );
    runner.check(
        optional_equals(empty.byte_size(), 0),
        "空张量字节数为0"
    );

    const std::size_t maximum =
        std::numeric_limits<std::size_t>::max();
    const TensorMetadata overflow(
        "huge",
        TensorShape({maximum, 2}),
        DataType::float32,
        TensorLayout::contiguous,
        Device(DeviceType::cpu, 0)
    );
    runner.check(
        !overflow.byte_size().has_value(),
        "溢出张量字节数为nullopt"
    );

    runner.expect_invalid_argument(
        [] {
            const TensorMetadata invalid(
                "",
                TensorShape({1}),
                DataType::float32,
                TensorLayout::contiguous,
                Device(DeviceType::cpu, 0)
            );
            static_cast<void>(invalid);
        },
        "拒绝空名称"
    );

    runner.expect_invalid_argument(
        [] {
            const TensorMetadata invalid(
                "wrong_rank",
                TensorShape({1, 80}),
                DataType::float32,
                TensorLayout::batch_time_feature,
                Device(DeviceType::cpu, 0)
            );
            static_cast<void>(invalid);
        },
        "拒绝layout和rank不匹配"
    );

    runner.expect_invalid_argument(
        [] {
            const TensorMetadata invalid(
                "wrong_dtype",
                TensorShape({1}),
                static_cast<DataType>(100),
                TensorLayout::contiguous,
                Device(DeviceType::cpu, 0)
            );
            static_cast<void>(invalid);
        },
        "拒绝非法dtype"
    );

    runner.expect_invalid_argument(
        [] {
            const TensorMetadata invalid(
                "wrong_layout",
                TensorShape({1}),
                DataType::float32,
                static_cast<TensorLayout>(100),
                Device(DeviceType::cpu, 0)
            );
            static_cast<void>(invalid);
        },
        "拒绝非法layout"
    );
}

} // namespace

int main() {
    TestRunner runner;

    test_data_types(runner);
    test_layouts(runner);
    test_devices(runner);
    test_shapes(runner);
    test_metadata(runner);

    return runner.finish();
}
