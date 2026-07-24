#include <iostream>

int main() {
    int* value = new int(100);

    delete value;
    value = nullptr;
    return 0;
}