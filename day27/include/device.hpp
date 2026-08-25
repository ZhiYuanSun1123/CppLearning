#ifndef DEVICE_HPP
#define DEVICE_HPP

#include <string>
#include <string_view>

enum class DeviceType {
    cpu,
    accelerator
};
class Device {
public:
    Device(
        DeviceType type,
        int index
    );
    [[nodiscard]] DeviceType type() const noexcept;
    [[nodiscard]] int index() const noexcept;

private:
    DeviceType type_;
    int index_;
};

[[nodiscard]] std::string_view to_string(
    DeviceType type
) noexcept;

[[nodiscard]] std::string to_string(
    const Device& device
);

[[nodiscard]] bool operator==(
    const Device& left,
    const Device& right
) noexcept;

#endif
