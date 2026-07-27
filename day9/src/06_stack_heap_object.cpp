#include <iostream>
#include <string>

class Trace {
public:
    Trace(const std::string& name)
        : name_(name) {
        std::cout
            << "Construct: "
            << name_
            << '\n';
    }

    ~Trace() {
        std::cout
            << "Destroy: "
            << name_
            << '\n';
    }

private:
    std::string name_;
};

void run() {
    Trace first("first");
    Trace second("second");
    {
        Trace inner("inner");
        std::cout << "Inside block\n";
    }
    std::cout << "Leaving run\n";
}

int main() {
    Trace stack_object("stack");

    Trace* heap_object =
        new Trace("heap");

    std::cout << "Before delete\n";

    delete heap_object;
    heap_object = nullptr;

    std::cout << "Before return\n";
    return 0;
}