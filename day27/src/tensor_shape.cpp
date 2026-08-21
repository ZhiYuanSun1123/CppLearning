#include"tensor_shape.hpp"
TensorShape::TensorShape()
    : dimensions_(std::vector<std::size_t>{}) {}
explicit TensorShape::TensorShape(
    std::vector<std::size_t> dimensions
) : dimensions_(dimensions) {
    for(auto num : dimensions_){
        if(num < 0)
            throw std::invalid_argument("数据不能为负数");
    }
}
TensorShape::TensorShape(
    std::initializer_list<std::size_t> dimensions
) : dimensions_(dimensions){
    for(auto num : dimensions_){
        if(num < 0)
            throw std::invalid_argument("数据不能为负数");
    }
}
[[nodiscard]] std::size_t TensorShape::rank() const noexcept{
    return dimensions_.size();
}
[[nodiscard]] std::size_t TensorShape::at(
    std::size_t axis
) const {
    return dimensions_.at(axis);
}
[[nodiscard]] bool TensorShape::is_scalar() const noexcept{
}
[[nodiscard]] std::optional<std::size_t> TensorShape::numel() const noexcept{
    if(rank()==0)
        return 1;
    else{
        std::size_t num = 1;
        const auto max_count = std::numeric_limits<std::size_t>::max();
        for(const auto& n : dimensions_){
            if(num!=0&&n>max_count/num)
                return std::nullopt;
            num*=n;
        }
        return num;
    }
}
[[nodiscard]] const std::vector<std::size_t>&
    TensorShape::dimensions() const noexcept {
    return dimensions_;
}
[[nodiscard]] bool TensorShape::is_scalar() const noexcept {
    if(rank()==0)
        return true;
    else
        return false;
}
[[nodiscard]] bool TensorShape::is_empty_tensor() const noexcept {
    if(numel()==0)
        return true;
    else
        return false;
}