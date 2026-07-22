#include <iostream>

int main() {
    const int size = 5;
    int* values = new int[size]{};

    for (int index = 0; index < size; ++index) {
        values[index] = index * 10;
    }

    delete[] values;
    values = nullptr;
    return 0;
}