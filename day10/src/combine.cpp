#include<iostream>
class Decoder {
public:
    Decoder() {
        std::cout << "Construct Decoder\n";
    }

    ~Decoder() {
        std::cout << "Destroy Decoder\n";
    }
};

class Pipeline {
public:
    Pipeline() {
        std::cout << "Construct Pipeline\n";
    }

    ~Pipeline() {
        std::cout << "Destroy Pipeline\n";
    }

private:
    Decoder decoder_;
};