#include<iostream>
int main(){
    int* leaked = new int(42);
    static_cast<void>(leaked);
    return 0;
}