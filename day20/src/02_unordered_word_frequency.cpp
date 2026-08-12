#include<iostream>
#include<unordered_map>
int main(){
    std::unordered_map<std::string,std::size_t> word;
    std::string input;
    std::getline(std::cin,input);
    for(
        auto iterator = input.begin();
        iterator != input.end();
    ) {
        int split = input.find(" ");
        if(word.find(input.substr(0,split))==word.end()){
            word.insert({
                input.substr(0,split),1
            });
        } else{
            word[input.substr(0,split)]++;
        }
        if(input.find(" ") == std::string::npos)
            iterator = input.end();
        input = input.substr(split+1,input.size());
        std::cout << input << std::endl;
    }
    for(const auto&[w,count] : word){
        std::cout << w << "->" << count << std::endl;
    }
}