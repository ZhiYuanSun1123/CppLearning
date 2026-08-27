#ifndef TENSOR_METADATA_HPP
#define TENSOR_METADATA_HPP

#include "data_type.hpp"
#include "device.hpp"
#include "tensor_layout.hpp"
#include "tensor_shape.hpp"

#include <cstddef>
#include <optional>
#include <string>

class TensorMetadata {
public:
    TensorMetadata(
        std::string name,
        TensorShape shape,
        DataType data_type,
        TensorLayout layout,
        Device device
    );

    [[nodiscard]] const std::string&
    name() const noexcept;

    [[nodiscard]] const TensorShape&
    shape() const noexcept;

    [[nodiscard]] DataType
    data_type() const noexcept;

    [[nodiscard]] TensorLayout
    layout() const noexcept;

    [[nodiscard]] const Device&
    device() const noexcept;

    [[nodiscard]] std::optional<std::size_t>
    byte_size() const noexcept;

private:
    std::string name_;
    TensorShape shape_;
    DataType data_type_;
    TensorLayout layout_;
    Device device_;
};

#endif
