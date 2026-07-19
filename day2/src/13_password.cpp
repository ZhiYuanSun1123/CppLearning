#include<iostream>
using namespace std;
int main(){
    string password = "2026";
    string input;
    int time;
    for(time = 0; time<3; time++){
        cin >> input;
        if(input == password){
            cout << "Access granted" << endl;
            break;
        }
    }
    if(time==3)
        cout << "Access denied" << endl;
    return 0;
}