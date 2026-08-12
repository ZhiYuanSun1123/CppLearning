#include<iostream>
#include<map>
int main(){
    std::map<int,std::string> query{
        {2,"tiny"},
        {4,"small"},
        {8,"medium"},
        {16,"large"},
        {24,"xlarge"}
    };
    std::cout << query.upper_bound(6)->first << std::endl;
    auto up = query.lower_bound(17);
    auto low = query.upper_bound(3);
    for(
        auto iterator = low;
        iterator != up;
        iterator++
    ) {
        std::cout << iterator->second << std::endl;
    }
}