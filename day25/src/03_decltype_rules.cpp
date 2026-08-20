#include<iostream>
int main(){
    int value;
    const int const_value = 1;
    int* pointer = &value;
    static_assert(
        std::is_same_v<decltype(value),int>
    );
    static_assert(
        std::is_same_v<decltype((value)),int&>
    );
    static_assert(
        std::is_same_v<decltype(const_value),const int>
    );
    static_assert(
        std::is_same_v<decltype((const_value)),const int&>
    );
    static_assert(
        std::is_same_v<decltype(value+1),int>
    );
    static_assert(
        std::is_same_v<decltype(*pointer),int&>
    );
}