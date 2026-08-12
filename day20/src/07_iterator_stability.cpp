#include<iostream>
#include<map>
#include<unordered_map>
int main(){
    std::map<int,std::string> map_insert{
        {0,"0"}
    };
    auto iterator_old = map_insert.begin();
    for(
        int i = 1;
        i <= 100;
        i++
    ) {
        map_insert.insert({
            i,std::to_string(i)
        });
    }
    std::cout << iterator_old->first << std::endl;
    std::unordered_map<int,std::string> unordered_map_insert{
        {0,"0"}
    };
    auto unordered_iterator = unordered_map_insert.begin();
    const int* pointer = &unordered_iterator->first;
    std::size_t num = unordered_map_insert.bucket_count();
    for(
        int i = 1;
        unordered_map_insert.bucket_count()==num;
        i++
    ) {
        unordered_map_insert.insert({
            i,std::to_string(i)
        });
    }
    auto iterator_new = unordered_map_insert.find(0);
    std::cout << *pointer << std::endl;
}