#include <iostream>
#include <string>

class Label {
public:
    Label(const std::string& text)
        : text_(text) {
    }

    std::string& text() {
        std::cout
            << "non-const version\n";
        return text_;
    }

    const std::string& text() const {
        std::cout
            << "const version\n";
        return text_;
    }

private:
    std::string text_;
};

int main() {
    Label normal("normal");
    const Label fixed("fixed");

    normal.text() = "changed";

    std::cout
        << normal.text()
        << '\n';

    std::cout
        << fixed.text()
        << '\n';

    return 0;
}