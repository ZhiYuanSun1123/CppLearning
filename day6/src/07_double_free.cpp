#include <iostream>

int main() {
    int* pointer = new int(42);

    delete pointer;
    pointer = nullptr;

    return 0;
}