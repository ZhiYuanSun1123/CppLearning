#include"simple_stack.hpp"
#include<iostream>
int main(){
    SimpleStack<int> stack;
    int num = 1;
    stack.push(num);
    stack.push(std::move(num));
}