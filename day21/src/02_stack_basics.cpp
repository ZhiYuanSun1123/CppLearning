#include<iostream>
#include<string>
#include<stack>
int main(){
    std::stack<std::string> s;
    s.push("load model");
    s.push("set batch=4");
    s.push("enable fp16");
    std::cout << s.top() << std::endl;
    while(!s.empty()){
        std::cout << s.top() << std::endl;
        s.pop();
        std::cout << s.size() << std::endl;
    }
}