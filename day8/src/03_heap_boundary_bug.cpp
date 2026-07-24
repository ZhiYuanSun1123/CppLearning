#include <iostream>

int main() {
    const int size = 5;
    int* values = new int[size]{};

    for (int index = 0; index < size; ++index) {
        values[index] = (index + 1) * 10;
    }

    for (int index = 0; index < size; ++index) {
        std::cout << values[index] << '\n';
    }

    delete[] values;
    values = nullptr;
    return 0;
}