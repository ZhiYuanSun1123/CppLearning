#ifndef METADATA_ERROR_HPP
#define METADATA_ERROR_HPP

#include <cstddef>
#include <optional>
#include <string>

enum class MetadataErrorCode {
    empty_name,
    invalid_dimension,
    invalid_data_type,
    invalid_layout,
    invalid_device,
    element_count_overflow,
    byte_size_overflow,
    rank_layout_mismatch
};

struct MetadataError {
    MetadataErrorCode code;
    std::string message;
    std::optional<std::size_t> axis;
};

#endif
