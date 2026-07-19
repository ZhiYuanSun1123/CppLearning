#include<iostream>
using namespace std;
int square(int value){
    return value*value;
}
int absolute_value(int value){
    if(value < 0)
        return -value;
    return value;
}
bool is_even(int value){
    if(value%2==0)
        return true;
    return false;
}
int main(){
    int num;
    cin >> num;
    cout << "Square: " << square(num) << endl;
    cout << "Absolute: " << absolute_value(num) << endl;
    cout << "Even: " << is_even(num) << endl;
}