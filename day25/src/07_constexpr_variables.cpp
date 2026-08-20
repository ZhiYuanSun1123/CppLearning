#include<iostream>
int main(){
    constexpr int sample_rate=16000;
    constexpr int frame_duration_ms=25;
    constexpr int hop_duration_ms=10;
    constexpr int channels=1;
    constexpr int frame_sample_count = sample_rate*frame_duration_ms*0.001;
    constexpr int hop_sample_count = sample_rate*hop_duration_ms*0.001;
    constexpr int second2_sampel_cout = sample_rate*2;
}