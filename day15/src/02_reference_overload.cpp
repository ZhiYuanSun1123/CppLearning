#include<iostream>
#include<utility>
class Token{};
void inspect(const Token& token){
    std::cout << "const lvalue reference" << std::endl;
}
void inspect(Token&& token){
    std::cout << "rvalue reference" << std::endl;
}
int main(){
    Token token;
    inspect(token);
    inspect(Token{});
    inspect(std::move(token));
    return 0;
}