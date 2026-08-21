#include"device.hpp"
#include<stdexcept>
Device::Device(
    DeviceType type,
    int index
) : type_(type),
    index_(index){
    if(index_ < 0)
        throw std::invalid_argument("index不能小于0");
    if(type == DeviceType::cpu && index_!=0)
        throw std::invalid_argument("cpu设备只允许编号0");
}
[[nodiscard]] DeviceType Device::type() const noexcept {
    return type_;
}
[[nodiscard]] int Device::index() const noexcept {
    return index_;
}