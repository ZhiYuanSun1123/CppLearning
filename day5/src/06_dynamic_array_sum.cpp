#include<iostream>
#include<iomanip>
using namespace std;
int main(){
    int size;
    cin >> size;
    if(size<1 || size >1000){
        cout << "Invalid Input" << endl;
        return 0;
    }
    int* values = new int[size]{};
    int sum = 0;
    double avg = 0;
    for(int i = 0; i < size; i++){
        cin >> values[i];
        sum += values[i];
    }
    avg = static_cast<double>(sum)/size;
    cout << "Sum: " << sum << endl;
    cout << "Avg: " << fixed << setprecision(2) << avg << endl;
    delete[] values;
    values = nullptr;
    return 0;
}