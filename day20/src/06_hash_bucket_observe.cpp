#include<iostream>
#include<unordered_map>
int main(){
    std::unordered_map<int,std::string> observe;
    std::size_t num = observe.bucket_count();
    std::size_t change_num = 0;
    std::size_t change_num_new = 0;
    for(
        int i = 0;
        i<50;
        i++
    ) {
        observe.insert({
            i,std::to_string(i)
        });
        if(observe.bucket_count()!=num){
            change_num ++;
            std::cout << observe.size() << std::endl;
            std::cout << observe.bucket_count() << std::endl;
            std::cout << observe.load_factor() << std::endl;
            std::cout << observe.max_load_factor() << std::endl;
        }
    }
    observe.clear();
    observe.reserve(50);
    num = observe.bucket_count();
    for(
        int i = 0;
        i<50;
        i++
    ) {
        observe.insert({
            i,std::to_string(i)
        });
        if(observe.bucket_count()!=num){
            change_num_new++;
            std::cout << observe.size() << std::endl;
            std::cout << observe.bucket_count() << std::endl;
            std::cout << observe.load_factor() << std::endl;
            std::cout << observe.max_load_factor() << std::endl;
        }
    }
    std::cout << "Old: " << change_num << std::endl;
    std::cout << "New: " << change_num_new << std::endl;
}