#include "tensor_layout.hpp"

std::string_view to_string(
    TensorLayout layout
) noexcept {
    switch (layout) {
        case TensorLayout::contiguous:
            return "contiguous";
        case TensorLayout::batch_channel_time:
            return "batch_channel_time";
        case TensorLayout::batch_time_feature:
            return "batch_time_feature";
    }

    return "unknown";
}

std::optional<std::size_t> expected_rank(
    TensorLayout layout
) noexcept {
    switch (layout) {
        case TensorLayout::contiguous:
            return std::nullopt;
        case TensorLayout::batch_channel_time:
        case TensorLayout::batch_time_feature:
            return std::size_t{3};
    }

    return std::nullopt;
}
