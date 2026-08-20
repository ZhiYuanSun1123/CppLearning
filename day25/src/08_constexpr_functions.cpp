#include"audio_math.hpp"
#include<iostream>
int main(){
    constexpr int sample_rate=16000;
    constexpr int seconds=25;
    constexpr int bytes=2;
    constexpr int channels=1;
    std::cout << milliseconds_to_samples(25) << std::endl;
    std::cout << samples_to_milliseconds(sample_rate) << std::endl;
    std::cout << clamp_sample_rate(0,200000,sample_rate) << std::endl;
    std::cout << bytes_for_pcm(channels,sample_rate,seconds,bytes) << std::endl;
}