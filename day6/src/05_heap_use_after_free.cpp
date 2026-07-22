#include <iostream>

int main() {
    int* pointer = new int(42);

    std::cout << *pointer << '\n';
    delete pointer;
    pointer = nullptr;
    return 0;
}