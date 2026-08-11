#include<iostream>
#include<vector>
int main(){
    std::vector<int> first;
    first.reserve(5);

    std::vector<int> second;
    second.resize(5);

    std::cout << "************fisrt************" << std::endl;
    std::cout << first.size() << std::endl;
    std::cout << first.capacity() << std::endl;
    try{
        std::cout << first.at(0) << std::endl;
    } catch(const std::out_of_range error){
        std::cout << "Out of Range: " << std::string(error.what()) << std::endl;
    }
    std::cout << "*****************************" << std::endl;
    std::cout << "************second***********" << std::endl;
    std::cout << second.size() << std::endl;
    std::cout << second.capacity() << std::endl;
    try{
        std::cout << second.at(0) << std::endl;
    } catch(const std::out_of_range error){
        std::cout << "Out of Range: " << std::string(error.what()) << std::endl;
    }
    std::cout << "*****************************" << std::endl;
}