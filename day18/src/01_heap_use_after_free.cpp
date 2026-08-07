#include<iostream>
#include<memory>
int main(){
    auto value = std::make_unique<int>(42);
    int* observed = value.get();
    value.reset();
    std::cout << *observed << std::endl;
    return 0;
}