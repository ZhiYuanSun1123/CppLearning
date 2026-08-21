enum class DeviceType{
    cpu,
    accelerator
};
class Device{
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