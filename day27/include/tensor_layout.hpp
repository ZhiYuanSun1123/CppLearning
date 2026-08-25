#ifndef TENSOR_LAYOUT_HPP
#define TENSOR_LAYOUT_HPP

#include <cstddef>
#include <optional>
#include <string_view>

enum class TensorLayout {
    contiguous,
    batch_channel_time,
    batch_time_feature
};

[[nodiscard]] std::string_view to_string(
    TensorLayout layout
) noexcept;

[[nodiscard]] std::optional<std::size_t> expected_rank(
    TensorLayout layout
) noexcept;

#endif
