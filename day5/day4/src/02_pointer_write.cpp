#include<iostream>
using namespace std;
int main(){
    int number = 10;
    int* pointer = &number;
    cout << "Before number: " << number << endl;
    cout << "Before pointer value: " << *pointer << endl;
    number = 99;
    cout << "After number: " << number << endl;
    cout << "After pointer value: " << *pointer << endl;
    return 0;
}