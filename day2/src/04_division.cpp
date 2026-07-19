#include<iostream>
using namespace std;
int main(){
    int a;
    int b;
    cin >> a >> b;
    int* result;
    cout << a/b << endl;
    if(b!=0){
        int q = a/b;
        int r = a%b;
        cout << "Quotient: " << q << endl;
        cout << "Remainder: " << r << endl;
    }
    else{
        cout << "Error: divisor cannot be zero";
    }
}