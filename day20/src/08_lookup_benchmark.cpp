#include<iostream>
#include<map>
#include<unordered_map>
#include<chrono>
int main(){
    std::map<int,int> map_time;
    std::unordered_map<int,int> unordered_map_time;
    std::cout << "===================map================" << std::endl;
    auto start = std::chrono::steady_clock::now();
    for(
        int i = 0;
        i <= 99999;
        i++
    ) {
        map_time.insert({
            i,i
        });
    }
    auto end = std::chrono::steady_clock::now();
    auto duration = end - start;
    std::cout << "Construct Duration: " << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << std::endl;
    start = std::chrono::steady_clock::now();
    map_time.find(10000);
    end = std::chrono::steady_clock::now();
    duration = end - start;
    std::cout << "Find Duration: " << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << std::endl;
    std::cout << "============unordered map============" << std::endl;
    start = std::chrono::steady_clock::now();
    for(
        int i = 0;
        i <= 99999;
        i++
    ) {
        unordered_map_time.insert({
            i,i
        });
    }
    end = std::chrono::steady_clock::now();
    duration = end - start;
    std::cout << "Construct Duration: " << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << std::endl;
    start = std::chrono::steady_clock::now();
    unordered_map_time.find(10000);
    end = std::chrono::steady_clock::now();
    duration = end - start;
    std::cout << "Find Duration: " << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << std::endl;
}