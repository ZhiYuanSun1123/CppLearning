#include<iostream>
#include<map>
int main(){
    std::map<std::string,int> sample_rates;
    sample_rates.insert({
        "meeting.wav",16000
    });
    sample_rates.insert({
        "music.wav",44100
    });
    sample_rates.insert({
        "noise.wav",48000
    });
    for(const auto&[path,sample_rate]:sample_rates){
        std::cout << sample_rates[path] << " ";
    } std::cout << std::endl;
    std::map<std::string,int>::iterator iterator =
        sample_rates.find("meeting.wav");
    std::map<std::string,int>::iterator iterator_no =
        sample_rates.find("me");
    sample_rates["meeting.wav"] = 100000;
    sample_rates.erase("noise.wav");
    std::cout << sample_rates.size() << std::endl;
}
