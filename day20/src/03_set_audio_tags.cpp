#include<iostream>
#include<set>
int main(){
    std::string input;
    std::getline(std::cin,input);
    std::set<std::string> tags;
    bool finish = false;
    while(
        !finish
    ) {
        std::size_t split = input.find(" ");
        tags.insert(input.substr(0,split));
        if(input.find(' ') == std::string::npos)
            finish = true;
        input = input.substr(split+1,input.size());
    }
    if(tags.find("speech") != tags.end())
        std::cout << *tags.find("speech") << std::endl;
    if(tags.find("unknown")== tags.end())
        std::cout << "No unknown" << std::endl;
    tags.erase("noise");
}