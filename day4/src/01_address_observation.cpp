#include<iostream>
using namespace std;
int main(){
    int number = 42;
    int* pointer = &number;

    cout << "number: " << number << endl;
    cout << "number position: " << &number << endl;
    cout << "pointer: " << pointer << endl;
    cout << "pointer position: " << &pointer << endl;
    cout << "*pointer :" << *pointer << endl;
    cout << "sizeof(number): " << sizeof(number) << endl;
    cout << "sizeof(pointer): " << sizeof(pointer) << endl;

    return 0;
}