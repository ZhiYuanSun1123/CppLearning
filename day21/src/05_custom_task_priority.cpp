#include<iostream>
#include<queue>
#include<vector>
struct InferenceTask{
    std::string request_id;
    int priority;
    std::size_t sequence;
};
int main(){
    auto cmp = [](const InferenceTask& left,const InferenceTask& right){
        return left.priority < right.priority;
    };
    std::priority_queue<
        InferenceTask,
        std::vector<InferenceTask>,
        decltype(cmp)
    > pq(cmp);
    pq.push(InferenceTask{
        "A",2,0
    });
    pq.push(InferenceTask{
        "B",5,1
    });
    pq.push(InferenceTask{
        "C",5,2
    });
    pq.push(InferenceTask{
        "D",1,3
    });
    while(!pq.empty()){
        std::cout << pq.top().request_id << std::endl;
        pq.pop();
    }
}