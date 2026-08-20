template <typename T>
constexpr const char* numeric_kind() {
    if constexpr (std::is_integral_v<T>) {
        return "integral";
    } else if constexpr (
        std::is_floating_point_v<T>
    ) {
        return "floating";
    } else {
        return "other";
    }
}