#include<iostream>
#include<algorithm>
#include<vector>
#include<iterator>
int main(){
    std::vector<int> list{1,2,3,4,5,6};
    for(
        auto iterator = list.begin();
        iterator != list.end();
        iterator ++
    ) {
        std::cout << *iterator << std::endl;
    } std::cout << std::endl;

    for(
        auto iterator = list.crbegin();
        iterator != list.crend();
        iterator ++
    ) {
        std::cout << *iterator << std::endl;
    } std::cout << std::endl;
    auto iterator = list.begin();
    iterator = std::next(std::next(std::next(iterator)));
    std::cout << *iterator << std::endl;
    std::cout << *std::prev(list.end()) << std::endl;
    iterator = list.begin();
    std::advance(iterator,3);
    std::cout << *iterator << std::endl;
    std::cout << std::distance(iterator,list.begin()) << std::endl;
}