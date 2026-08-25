#include "data_type.hpp"

[[nodiscard]] std::string_view to_string(
    DataType type
) noexcept {
    switch (type) {
        case DataType::float32:
            return "float32";
        case DataType::float16:
            return "float16";
        case DataType::int32:
            return "int32";
        case DataType::int8:
            return "int8";
        case DataType::boolean:
            return "boolean";
    }
    return "unknown";
}
[[nodiscard]] std::optional<std::size_t> bytes_per_element(
    DataType type
) noexcept {
    switch (type) {
        case DataType::float32:
            return std::size_t{4};
        case DataType::float16:
            return std::size_t{2};
        case DataType::int32:
            return std::size_t{4};
        case DataType::int8:
            return std::size_t{1};
        case DataType::boolean:
            return std::size_t{1};
    }

    return std::nullopt;
}
