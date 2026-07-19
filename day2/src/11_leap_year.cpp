#include<iostream>
using namespace std;
int main(){
    int year;
    cin >> year;
    if(year%400==0 || (year%4==0 && year%100!=0))
        cout << "Leap Year" << endl;
    else
        cout << "Year" << endl;
    return 0;
}