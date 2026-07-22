#include <iostream>

int main() {
    const int size = 3;
    int* values = new int[size]{10, 20, 30};

    std::cout << "Array contents: ";

    for (int index = 0; index < size; ++index) {
        std::cout << values[index] << ' ';
    }

    std::cout << "\nEnter an index: ";

    int index = 0;
    std::cin >> index;

    // 这里没有检查index是否合法，故意保留内存安全问题
    std::cout << "values[" << index << "] = "
              << values[index]
              << '\n';

    delete[] values;
    values = nullptr;

    return 0;
}