#include<iostream>
using namespace std;
void observe_local(int call_number){
    int* values = new int(call_number);
    cout << "Values: " << *values << endl;
    cout << "Position: " << values << endl; 
    delete values;
    values = nullptr;
}
int main(){
    int call_number = 42;
    for(int i = 0; i<3; i++){
        cout << "Index: " << i << endl;
        observe_local(call_number);
    }
    return 0;
}