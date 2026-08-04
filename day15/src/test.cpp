#include<iostream>
#include<utility>
void inspect(int&){
    std::cout << "lvalue overload" << std::endl;
}
void inspect(int&&){
    std::cout << "rvalue overload" << std::endl;
}
int main(){
    int i = 1;
    inspect(i);
    inspect(1);
    return 0;
}