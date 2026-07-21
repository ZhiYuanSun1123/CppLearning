#include<iostream>
using namespace std;
int main(){
    int num;
    cin >> num;
    int* pointer = new int(num);
    cout << "Values: " << *pointer << endl;
    cout << "Position: " << pointer << endl;
    cout << "Pointer Position: " << &pointer << endl;
    delete pointer;
    pointer = nullptr;
    return 0;
}