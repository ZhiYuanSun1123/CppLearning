#include<algorithm>
#include<vector>
#include<iostream>
int main(){
    std::vector<double> seconds{
        0.5, -1.0, 2.25, 0.0, 3.5
    };
    seconds.erase(
        remove_if(
            seconds.begin(),
            seconds.end(),
            [](double value){
               return value < 0;
            }
        ),
        seconds.end()
    );
    for(auto iterator : seconds){
        std::cout << iterator << " ";
    } std::cout << std::endl;
    std::vector<double> mill;
    mill.resize(seconds.size());
    std::transform(
        seconds.begin(),
        seconds.end(),
        mill.begin(),
        [](double value){
            return value*1000;
        }
    );
    bool all_positive = std::all_of(
        mill.begin(),
        mill.end(),
        [](double value){
            return value>=0;
        }
    );
    std::cout << "结果非负: " << all_positive << std::endl;
}