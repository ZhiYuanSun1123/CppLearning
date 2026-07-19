#include<iostream>
using namespace std;
int main(){
    int num;
    int temp;
    int sum = 0;
    int digits = 0;
    int reversed = 0;
    cin >> num;
    temp = num;
    if(num==0){
        cout << "Invalid input" << endl;
        return 0;
    }
    while(temp != 0){
        reversed*=10;
        sum+=temp%10;
        reversed+=temp%10;
        digits+=1;
        temp/=10;
    }
    cout << "Digits: " << digits << endl;
    cout << "Digit sum: " << sum << endl;
    cout << "Reversed: " << reversed << endl;
    return 0;
}