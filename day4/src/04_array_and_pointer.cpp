#include<iostream>
using namespace std;
int main(){
    int values[4] = {7,14,21,28};
    int* pointer = values;
    cout << "values: " << values << endl;
    cout << "&values: " << &values[0] << endl;
    cout << "values[0]: " << values[0] << endl;
    cout << "*pointer: " << *pointer << endl;
    cout << "values[1]: " << values[1] << endl;
    cout << "*(pointer+1): " << *(pointer+1) << endl;
    cout << "sizeof(values): " << sizeof(values) << endl; 
    cout << "sizeof(pointer): " << sizeof(pointer) << endl;
    return 0;
}