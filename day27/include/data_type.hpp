#ifndef DATA_TYPE_HPP
#define DATA_TYPE_HPP

#include <cstddef>
#include <optional>
#include <string_view>

enum class DataType {
    float32,
    float16,
    int32,
    int8,
    boolean
};

[[nodiscard]] std::string_view to_string(
    DataType type
) noexcept;

[[nodiscard]] std::optional<std::size_t> bytes_per_element(
    DataType type
) noexcept;

#endif
