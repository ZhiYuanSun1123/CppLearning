#include<iostream>
using namespace std;
int main(){
    int value[5] = {10,20,30,40,50};
    for(int i = 0;i < 5;i++){
        cout << "Index: " << i << endl;
        cout << "Value: " << value[i] << endl;
        cout << "Position: " << &value[i] << endl;
    }
    cout << "Int Size: " << sizeof(int) << endl;
    cout << "Values Size: " << sizeof(value) << endl;
    cout << "Num: " << sizeof(value) / sizeof(value[0]) << endl;
    return 0;
}