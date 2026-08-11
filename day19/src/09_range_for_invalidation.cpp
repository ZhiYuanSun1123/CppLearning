#include<iostream>
#include<vector>
int main(){
    std::vector<int> values{
        1,2,3,4,5,6,7,8,9,10
    };
    for(const int& value : values){
        values.push_back(value);
    }
    for(const int& value : values){
        std::cout << value << std::endl;
    }
}