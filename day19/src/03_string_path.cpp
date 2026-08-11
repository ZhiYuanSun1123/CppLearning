#include<iostream>
#include<string>
int main(){
    std::string path;
    std::cin >> path;
    std::cout << ((path.find(".wav")!=std::string::npos)? 0 : 1 )<< std::endl;
    auto sample = path;
    for(auto pos = sample.find("/");pos!=std::string::npos;){
        sample = sample.substr(pos+1,path.size());
        pos = sample.find("/");
    }
    std::cout << sample << std::endl;
    std::cout << "Processing: " + sample << std::endl;
    std::cout << path.size() << std::endl;
    std::cout << path.capacity() << std::endl;
    std::cout << path.c_str() << std::endl;
    path = path.replace(3,1,"c");
    std::cout << path.c_str() << std::endl;
}