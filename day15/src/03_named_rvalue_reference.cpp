#include<iostream>
#include<utility>
class Token{};
void inspect(const Token&){
    std::cout << "lvalue overload" << std::endl;
}
void inspect(Token&&){
    std::cout << "rvalue overload" << std::endl;
}
void receive(Token&& token){
    inspect(token);
    inspect(std::move(token));
}
int main(){
    receive(Token{});
    return 0;
}