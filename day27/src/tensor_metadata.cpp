#include "tensor_metadata.hpp"

#include <limits>
#include <stdexcept>
#include <utility>

TensorMetadata::TensorMetadata(
    std::string name,
    TensorShape shape,
    DataType data_type,
    TensorLayout layout,
    Device device
) : name_(std::move(name)),
    shape_(std::move(shape)),
    data_type_(data_type),
    layout_(layout),
    device_(device) {
    if (name_.empty()) {
        throw std::invalid_argument("名称不能为空");
    }

    if (!bytes_per_element(data_type_).has_value()) {
        throw std::invalid_argument("无法识别数据类型");
    }

    switch (layout_) {
        case TensorLayout::contiguous:
            break;
        case TensorLayout::batch_channel_time:
        case TensorLayout::batch_time_feature: {
            const auto required_rank = expected_rank(layout_);
            if (!required_rank.has_value() ||
                shape_.rank() != *required_rank) {
                throw std::invalid_argument(
                    "张量布局要求rank为3"
                );
            }
            break;
        }
        default:
            throw std::invalid_argument("无法识别张量布局");
    }
}

const std::string& TensorMetadata::name() const noexcept {
    return name_;
}

const TensorShape& TensorMetadata::shape() const noexcept {
    return shape_;
}

DataType TensorMetadata::data_type() const noexcept {
    return data_type_;
}

TensorLayout TensorMetadata::layout() const noexcept {
    return layout_;
}

const Device& TensorMetadata::device() const noexcept {
    return device_;
}

std::optional<std::size_t>
TensorMetadata::byte_size() const noexcept {
    const auto element_bytes =
        bytes_per_element(data_type_);
    const auto element_count = shape_.numel();

    if (!element_bytes.has_value() ||
        !element_count.has_value()) {
        return std::nullopt;
    }

    if (*element_count == 0) {
        return std::size_t{0};
    }

    const std::size_t maximum =
        std::numeric_limits<std::size_t>::max();

    if (*element_bytes > maximum / *element_count) {
        return std::nullopt;
    }

    return *element_bytes * *element_count;
}
