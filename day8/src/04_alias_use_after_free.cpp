#include <iostream>

int main() {
    int* owner = new int(42);
    int* observer = owner;

    std::cout << *observer << '\n';
    delete owner;
    owner = nullptr;
    observer = nullptr;

    return 0;
}