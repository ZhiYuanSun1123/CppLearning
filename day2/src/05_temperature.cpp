#include<iostream>
#include<iomanip>
using namespace std;
int main(){
    double c;
    double f;
    string feeling;
    cin >> c;
    f = c*9/5 + 32;
    if(c<0)
        feeling = "Freezing";
    else if(c>=0 && c<15)
        feeling = "Cold";
    else if(c>=15 && c<28)
        feeling = "Comfortable";
    else
        feeling = "Hot";
    cout << "Fahrenheit: " << fixed << setprecision(2) << f << endl;
    cout << "Feeling: " << feeling << endl;
}