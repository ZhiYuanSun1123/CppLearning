#include<iostream>
#include<iomanip>
using namespace std;
int main(){
    int class_1;
    int class_2;
    int class_3;
    cin >> class_1 >> class_2 >> class_3;
    int toal_score = class_1 + class_2 + class_3;
    float Average = static_cast<double>(toal_score) / 3;
    cout << "Toal " << toal_score << endl;
    cout << "Average " << std::fixed << std::setprecision(2) << Average;
    return 0;
}