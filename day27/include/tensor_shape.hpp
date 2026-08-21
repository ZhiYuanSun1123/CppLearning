#ifndef TENSOR_SHAPE_HPP
#define TENSOR_SHAPE_HPP

#include<cstddef>
#include<initializer_list>
#include<optional>
#include<vector>
class TensorShape{
public:
    TensorShape();
    explicit TensorShape(
        std::vector<std::size_t> dimensions
    );
    TensorShape(
        std::initializer_list<std::size_t> dimensions
    );
    [[nodiscard]] std::size_t rank() const noexcept;
    [[nodiscard]] bool is_scalar() const noexcept;
    [[nodiscard]] bool is_empty_tensor() const noexcept;
    [[nodiscard]] const std::vector<std::size_t>&
        dimensions() const noexcept;
    [[nodiscard]] std::size_t at(
        std::size_t axis
    ) const;
    [[nodiscard]] std::optional<std::size_t>
        numel() const noexcept;
private:
    std::vector<std::size_t> dimensions_;
};
#endif