#include "device.hpp"

#include <stdexcept>

Device::Device(
    DeviceType type,
    int index
) : type_(type), index_(index) {
    switch (type_) {
        case DeviceType::cpu:
        case DeviceType::accelerator:
            break;
        default:
            throw std::invalid_argument(
                "无法识别设备类型"
            );
    }

    if (index_ < 0) {
        throw std::invalid_argument("index不能小于0");
    }

    if (type_ == DeviceType::cpu && index_ != 0) {
        throw std::invalid_argument("cpu设备只允许编号0");
    }
}

DeviceType Device::type() const noexcept {
    return type_;
}

int Device::index() const noexcept {
    return index_;
}

std::string_view to_string(DeviceType type) noexcept {
    switch (type) {
        case DeviceType::cpu:
            return "cpu";
        case DeviceType::accelerator:
            return "accelerator";
    }

    return "unknown";
}

std::string to_string(const Device& device) {
    return std::string(to_string(device.type())) +
           ':' +
           std::to_string(device.index());
}

bool operator==(
    const Device& left,
    const Device& right
) noexcept {
    return left.type() == right.type() &&
           left.index() == right.index();
}
