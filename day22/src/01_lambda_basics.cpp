#include<iostream>
int main(){
    auto detect = [](int value)->bool{
        return value%2 == 0;
    };
    auto caculate_2 = [](int value)->int{
        return value*value;
    };
    auto is_valid = [](int length)->bool{
        return length>0;
    };
    auto transform = [](double time)->long long{
        return static_cast<long long>(time);
    };
}