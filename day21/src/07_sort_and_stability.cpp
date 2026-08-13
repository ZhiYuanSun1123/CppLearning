#include<iostream>
#include<algorithm>
#include<vector>
struct InferenceTask{
    std::string id;
    int priority;
};
int main(){
    std::vector<InferenceTask> list;
    std::sort(
        list.begin(),
        list.end(),
        [](const InferenceTask& left,const InferenceTask& right){
            return left.priority > right.priority;
        }
    );
    for(auto iterator : list){
        std::cout << iterator.id << " ";
    } std::cout << std::endl;
    std::stable_sort(
        list.begin(),
        list.end(),
        [](const InferenceTask& left, const InferenceTask& right){
            return left.priority > right.priority;
        }
    );
    for(auto iterator : list){
        std::cout << iterator.id << " ";
    } std::cout << std::endl;
    std::cout << std::is_sorted(
        list.begin(),
        list.end(),
        [](const InferenceTask& left, const InferenceTask& right){
            return left.priority > right.priority;
        }
    ) << std::endl;

}
