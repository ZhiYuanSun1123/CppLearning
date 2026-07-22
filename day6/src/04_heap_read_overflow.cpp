#include <iostream>

int main() {
    const int size = 4;
    int* values = new int[size]{7, 14, 21, 28};

    std::cout << values[size-1] << '\n';

    delete[] values;
    values = nullptr;
    return 0;
}