#include"tensor_metadata.hpp"
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
    device_(device){
    if(name_.length()==0)
        throw std::invalid_argument("名称不能为空");
}
[[nodiscard]] const std::string& TensorMetadata::name() const noexcept{
    return name_;
}
[[nodiscard]] const TensorShape& TensorMetadata::shape() const noexcept {
    return shape_;
}
[[nodiscard]] DataType TensorMetadata::data_type() const noexcept{
    return data_type_;
}

[[nodiscard]] TensorLayout TensorMetadata::layout() const noexcept{
    return layout_;
}
[[nodiscard]] const Device& TensorMetadata::device() const noexcept {
    return device_;
}
[[nodiscard]] std::optional<std::size_t> TensorMetadata::byte_size() const noexcept{
    auto byte = bytes_per_element(data_type_);
    auto count = shape_.numel();
    if(byte.has_value()&&count.has_value()){
        if(byte.value() > std::numeric_limits<std::size_t>::max() / count.value())
            return std::nullopt;
        return byte.value()*count.value();
    } else
        return std::nullopt;
}