#include<iostream>
using namespace std;
bool safe_read(const int* pointer, int& output);
int main(){
    int number = 25;
    int result = 10;
    const int* valid_pointer = &number;
    const int* null_pointer = nullptr;
    safe_read(valid_pointer,result);
    cout << result << endl;
    result = 10;
    safe_read(null_pointer,result);
    cout << result << endl;
    return 0;
}
bool safe_read(const int* pointer,int& output){
    if(pointer==nullptr)
        return false;
    else{
        output = *pointer;
        return true;
    }
}