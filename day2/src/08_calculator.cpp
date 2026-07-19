#include<iostream>
#include<iomanip>
using namespace std;
int main(){
    double num_1;
    double num_2;
    char ca;
    double result;
    cin >> num_1 >> ca >> num_2;
    switch(ca){
        case '+':
            result = num_1 + num_2;
            break;
        case '-':
            result = num_1 - num_2;
            break;
        case '*':
            result = num_1 * num_2;
            break;
        case '/':
            result = num_1 / num_2;
            break;
    }
    cout << "Result: " << fixed << setprecision(2) << result << endl;
    return 0;
}