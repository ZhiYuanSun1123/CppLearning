#include <iostream>

int main() {
    int* values = new int[5]{};
    values[0] = 10;

    std::cout << values[0] << '\n';

    delete[] values;
    return 0;
}