#include<iostream>
using namespace std;
int main(){
    int num;
    cin >> num;
    cout << "Sign: ";
    if(num > 0)
        cout << "positive" << endl;
    else if(num < 0)
        cout << "negitive" << endl;
    else
        cout << "zero" << endl;
    cout << "Partiy: ";
    if(num % 2 == 0)
        cout << "even" << endl;
    else
        cout << "odd" << endl;
    cout << "Divisible by both 3 and 5: ";
    if(num%3==0 && num%5==0)
        cout << "true" << endl;
    else
        cout << "false" << endl;
    return 0;
}