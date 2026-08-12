#include<iostream>
#include<map>
int main(){
    std::map<std::string,int> counts {{
        "qwen",3
    }};
    std::cout << "Size: " << counts.size() << std::endl;
    std::cout << "Counts[\"missing\"]: " << counts["missing"] << std::endl;
    std::cout << "Size: " << counts.size() << std::endl;
    if(counts.find("another_missing")!=counts.end()){
        std::cout << "First: " << counts.find("another_missing")->first << std::endl;
        std::cout << "Second: " << counts.find("another_missing")->second << std::endl;
    }
    else
        std::cout << "No another missing" << std::endl;
    std::cout << "Size: " << counts.size() << std::endl;
    std::cout << "at(\"missing\"): " << counts.at("missing") << std::endl;
    try{
        counts.at("unknown");
    } catch(const std::out_of_range error){
        std::cout << "没有内容" << std::endl;
    }
}