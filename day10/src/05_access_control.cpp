#include <iostream>
#include <string>

class Base {
public:
    Base()
        : public_value_(1),
          protected_value_(2),
          private_value_(3) {
    }

    int private_value() const {
        return private_value_;
    }

    int public_value_;

protected:
    int protected_value_;

private:
    int private_value_;
};

class Derived : public Base {
public:
    void print() const {
        std::cout
            << public_value_
            << '\n';

        std::cout
            << protected_value_
            << '\n';

        std::cout
            << private_value()
            << '\n';
    }
};

int main() {
    Derived object;
    object.print();

    std::cout
        << object.public_value_
        << '\n';
    // std::cout << object.protected_value_ << std::endl;
    // std::cout << object.private_value_ << std::endl;
    return 0;
}