#include<iostream>
using namespace std;
int& func(){
    static int a = 10;
    return a;
}
int main(){
    func()=100;
    int ref = func();
    cout << ref;
}