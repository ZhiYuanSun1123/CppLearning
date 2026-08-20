#include<iostream>
#include<optional>
int main(){
    std::optional<int> test;
    test = std::nullopt;
    test = 16;
    if(test.has_value()){
        std::cout << test.value() << std::endl;
    }
    std::cout << *test << std::endl;
    test.reset();
    if(test.has_value()){
        std::cout << "have value " << std::endl;
    }
    std::cout << test.value_or(160000) << std::endl;
    test.emplace(1);
    std::cout << test.value() << std::endl;
}