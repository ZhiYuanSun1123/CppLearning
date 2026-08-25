#include "tensor_shape.hpp"

#include <algorithm>
#include <limits>
#include <utility>

TensorShape::TensorShape() = default;

TensorShape::TensorShape(
    std::vector<std::size_t> dimensions
) : dimensions_(std::move(dimensions)) {}

TensorShape::TensorShape(
    std::initializer_list<std::size_t> dimensions
) : dimensions_(dimensions) {}

std::size_t TensorShape::rank() const noexcept {
    return dimensions_.size();
}

bool TensorShape::is_scalar() const noexcept {
    return dimensions_.empty();
}

bool TensorShape::is_empty_tensor() const noexcept {
    return std::any_of(
        dimensions_.begin(),
        dimensions_.end(),
        [](std::size_t dimension) {
            return dimension == 0;
        }
    );
}

const std::vector<std::size_t>&
TensorShape::dimensions() const noexcept {
    return dimensions_;
}

std::size_t TensorShape::at(
    std::size_t axis
) const {
    return dimensions_.at(axis);
}

std::optional<std::size_t>
TensorShape::numel() const noexcept {
    if (is_empty_tensor()) {
        return std::size_t{0};
    }

    std::size_t count = 1;
    const std::size_t maximum =
        std::numeric_limits<std::size_t>::max();

    for (const std::size_t dimension : dimensions_) {
        if (dimension > maximum / count) {
            return std::nullopt;
        }

        count *= dimension;
    }

    return count;
}

bool operator==(
    const TensorShape& left,
    const TensorShape& right
) noexcept {
    return left.dimensions() == right.dimensions();
}
