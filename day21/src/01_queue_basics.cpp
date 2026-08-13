#include<iostream>
#include<queue>
int main(){
    std::queue<std::string> q;
    q.push("req-001");
    q.push("req-002");
    q.push("req-003");
    std::cout << q.front() << std::endl;
    std::cout << q.back() << std::endl;
    std::cout << q.size() << std::endl;
    while(!q.empty()){
        std::cout << q.front() << std::endl;
        q.pop();
    }
}