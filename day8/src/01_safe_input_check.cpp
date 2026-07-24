#include <iostream>

bool is_safe(const int* data, int size) {
    if (data == nullptr || size <= 0) {
        return false;
    }

    return true;
}

int main() {
    const int values[] = {10, 20, 30};

    std::cout << std::boolalpha;
    std::cout << is_safe(values, 3) << '\n';
    std::cout << is_safe(nullptr, 3) << '\n';
    std::cout << is_safe(values, 0) << '\n';
    std::cout << is_safe(nullptr, 0) << '\n';

    return 0;
}