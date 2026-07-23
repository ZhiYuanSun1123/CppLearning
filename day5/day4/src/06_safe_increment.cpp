#include<iostream>
using namespace std;
bool safe_increment(int* pointer);
int main(){
    int number = 10;
    int* pointer = &number;
    int* nullpointer = nullptr;
    safe_increment(pointer);
    cout << pointer << endl;
    safe_increment(nullpointer);
    cout << nullpointer << endl;
}
bool safe_increment(int* pointer){
    if(pointer==nullptr)
        return false;
    else{
        pointer+=1;
        return true;
    }
}