#include<iostream>
#include<iomanip>
using namespace std;
long long array_sum(const int* data, int size);
double array_average(const int* data, int size);
bool is_safe(const int* data, int size);
int main(){
    const int values[] = {80,91,76,88,95};
    int size = sizeof(values) / sizeof(int);
    cout << "Sum: "
         << array_sum(values,size)
         << endl;
    cout << "Average: "
         << fixed 
         << setprecision(1) 
         << array_average(values,size) 
         << endl;
    return 0;
}
bool is_safe(const int* data, int size){
    if(data == nullptr || size <= 0)
        return false;
    else
        return true;
}
long long array_sum(const int* data, int size){
    if(!is_safe(data,size)){
        cout << "Invalid Input" << endl;
        return 0;
    }
    long long sum = 0;
    for(int i = 0; i < size; i++){
        sum += data[i];
    }
    return sum;
}
double array_average(const int* data, int size){
    if(!is_safe(data,size)){
        cout << "Invalid Input" << endl;
        return 0;
    }
    long long sum = 0;
    for(int i = 0; i < size; i++){
        sum += data[i];
    }
    double average = static_cast<double>(sum)/size;
    return average;
}