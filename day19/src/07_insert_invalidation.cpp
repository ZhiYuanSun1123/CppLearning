#include<iostream>
#include<vector>
int main(){
    std::vector<int> values{
        1,2,3,4,5
    };
    int sample = 1;
    int* pos_sample = &sample;
    auto pos_time = values.begin();
    pos_time+=3;
    values.insert(pos_time,sample);
}