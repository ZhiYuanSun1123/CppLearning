#include<iostream>
using namespace std;
int main(){
    long long time;
    cin >> time;
    int hour = time/3600;
    int min = (time - 3600*hour)/60;
    int sec = time - 3600*hour - 60*min;
    cout << hour << " hour(s) " << min << " minute(s) " << sec << " second(s) ";
    return 0;
}