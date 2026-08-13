#include<iostream>
#include<vector>
#include<algorithm>
int main(){
    std::vector<int> latencies{
        25, 40, 120, 55, 120, 10
    };
    auto iterator = std::find(latencies.begin(),latencies.end(),120);
    iterator = std::find(latencies.begin(),latencies.end(),999);
    if(iterator == latencies.end())
        std::cout << "没找到" << std::endl;
    iterator = std::find_if(
        latencies.begin(),
        latencies.end(),
        [](int value){
            return value > 100;
        }
    );
    std::cout << *iterator << std::endl;
    const auto count = std::count(
        latencies.begin(),
        latencies.end(),
        120
    );
    const auto positive_count = std::count_if(
        latencies.begin(),
        latencies.end(),
        [](int value){
            return value <= 50;
        }
    );
    std::cout << "<=50: " << positive_count << std::endl;
    bool all_positive = all_of(
        latencies.begin(),
        latencies.end(),
        [](int value){
            return value >= 0;
        }
    );
    std::cout << "非负: " << all_positive << std::endl;
    bool have_more_100 = any_of(
        latencies.begin(),
        latencies.end(),
        [](int value){
            return value > 100;
        }
    );
    std::cout << "超过100: " << have_more_100 << std::endl;
    bool have_negitive = none_of(
        latencies.begin(),
        latencies.end(),
        [](int value){
            return value<0;
        }
    );
    std::cout << "不存在负数: " << have_negitive << std::endl;
}