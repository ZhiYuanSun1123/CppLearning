#include<iostream>
#include<vector>
int main(){
    std::vector<int> values{
        1,2,3,4,5,6,7,8,9,10
    };
    for(
        auto iterator = values.begin();
        iterator != values.end();
    ) {
        if(*iterator%2==0){
            iterator = values.erase(iterator);
        } else{
            iterator++;
        }
    }
    for(
        auto iterator = values.begin();
        iterator != values.end();
        iterator ++
    ) {
        std::cout << *iterator << std::endl;
    }
    std::cout << values.size() << std::endl;
}