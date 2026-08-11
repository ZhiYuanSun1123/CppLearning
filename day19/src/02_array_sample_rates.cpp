#include<iostream>
#include<vector>
int main(){
    std::array<int,4> arr{
        16000,22050,44100,48000
    };
    for(
        auto iterator = arr.cbegin();
        iterator != arr.cend();
        ++iterator
    ) {
        std::cout << *iterator << " ";
    } std::cout << std::endl;
    std::cout << arr.at(1) << std::endl;
    std::cout << arr.size() << std::endl;
    std::cout << arr.front() << std::endl;
    std::cout << arr.back() << std::endl;
    auto arr2 = arr;
}