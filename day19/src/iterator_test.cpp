#include<iostream>
#include<vector>
int main(){
    std::vector<int> values{1,2,3,4,5,6};
    auto iterator = values.begin();
    std::cout << *iterator << std::endl;
}