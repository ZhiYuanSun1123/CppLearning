#include <iostream>

int main() {
    int* values = new int[5]{};
    values[0] = 10;

    delete[] values;
    return 0;
}