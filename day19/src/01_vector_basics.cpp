#include<iostream>
#include<vector>
int main(){
    std::vector<int> values{
        7,14,21,28
    };
    std::cout << values.size() << std::endl;
    std::cout << values.capacity() << std::endl;
    std::cout << values.empty() << std::endl;
    values.push_back(35);
    values.push_back(42);
    try{
        values.at(values.size());
    } catch(const std::out_of_range error){
        std::cout << "Out of Range: " << std::string(error.what()) << std::endl;
    }
}