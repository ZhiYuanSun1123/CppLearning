#include<iostream>
#include<functional>
#include<queue>
#include<vector>
int main(){
    std::priority_queue<int> pq;
    pq.push(3);
    pq.push(10);
    pq.push(5);
    pq.push(1);
    pq.push(8);
    while(!pq.empty()){
        std::cout << pq.top() << std::endl;
        pq.pop();
        std::cout << pq.size() << std::endl;
    }
}