#include <iostream>

int main() {
    int* owner = new int(99);
    int* observer = owner;
    std::cout << *observer << '\n';
    delete owner;
    owner = nullptr;
    return 0;
}