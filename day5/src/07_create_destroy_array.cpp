#include<iostream>
using namespace std;
int* create_sequence(int size){
    if(size<=0)
        return nullptr;
    int* pointer = new int[size]{};
    for(int i = 0; i<size; i++){
        cin >> pointer[i];
    }
    return pointer;
}
void destroy_array(int*& data){
    delete[] data;
    data = nullptr;
}
int main(){
    int* pointer = nullptr;
    pointer = create_sequence(5);
    for(int i = 0; i < 5; i++)
        cout << pointer[i] << endl;
    destroy_array(pointer);
    if(pointer == nullptr)
        cout << "Null Pointer" << endl;
    destroy_array(pointer);
    return 0;
}