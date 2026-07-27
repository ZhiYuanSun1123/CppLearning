#include <iostream>

class OrderDemo {
public:
    OrderDemo(int value)
        : second_(value),
          first_(second_) {
        std::cout
            << "first = "
            << first_
            << ", second = "
            << second_
            << '\n';
    }

private:
    int second_;
    int first_;
};

int main() {
    OrderDemo demo(42);
    return 0;
}