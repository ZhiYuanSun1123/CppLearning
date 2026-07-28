#include <iostream>

class Base {
public:
    Base() {
        std::cout << "1 Base\n";
    }

    ~Base() {
        std::cout << "6 ~Base\n";
    }
};

class Component {
public:
    Component() {
        std::cout << "2 Component\n";
    }

    ~Component() {
        std::cout << "5 ~Component\n";
    }
};

class Derived : public Base {
public:
    Derived() {
        std::cout << "3 Derived\n";
    }

    ~Derived() {
        std::cout << "4 ~Derived\n";
    }

private:
    Component component1_;
    Component component2_;
};

int main() {
    Derived object;
    return 0;
}