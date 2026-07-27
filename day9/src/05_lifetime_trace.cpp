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
        return;
        Trace inner("inner");
        std::cout << "Inside block\n";
    }
    std::cout << "Leaving run\n";
}

int main() {
    run();
    std::cout << "Leaving main\n";
    return 0;
}