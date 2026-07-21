#include<iostream>
using namespace std;
int* copy_array(const int* source, int size){
    if(source == nullptr || size <= 0)
        return nullptr;
    int* pointer = new int[size]{};
    for(int i = 0; i < size; i++){
        pointer[i] =source[i];
    }
    return pointer;
}
int main(){
    const int source[] = {7,14,21,28,35};
    int size = sizeof(source)/sizeof(int);
    int* pointer = copy_array(source,size);
    pointer[0] = 10000;
    cout << source[0] << endl;
    delete[] pointer;
    pointer = nullptr;
    return 0;
}