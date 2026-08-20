#include<iostream>
#include<type_traits>
int main(){
    const int num = 1;
    auto num_auto = num;
    static_assert(
        std::is_same_v<decltype(num_auto),int>
    );
    auto& num_auto_reference = num;
    static_assert(
        std::is_same_v<decltype(num_auto_reference),const int&>
    );
    int num_no = 1;
    auto& num_reference = num_no;
    static_assert(
        std::is_same_v<decltype(num_reference),int&>
    );
    const int* num_pointer = &num_no;
    auto num_auto2 = num_pointer;
    static_assert(
        std::is_same_v<decltype(num_auto2),const int*>
    );
}
