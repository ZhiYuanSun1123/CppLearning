#include<iostream>
#include <type_traits>
#include<vector>
int main(){
    auto num = 1;
    static_assert(
        std::is_same_v<decltype(num),int>
    );
    auto num2 = 1.0;
    static_assert(
        std::is_same_v<decltype(num2),double>
    );
    auto char_str = 'r';
    static_assert(
        std::is_same_v<decltype(char_str),char>
    );
    auto str = std::string("string");
    static_assert(
        std::is_same_v<decltype(str),std::string>
    );
    std::vector<int> list = {1,2,3,4,5};
    auto iterator = list.begin();
    static_assert(
        std::is_same_v<decltype(iterator),std::vector<int>::iterator>
    );
}