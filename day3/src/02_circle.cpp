#include<iostream>
#include<iomanip>
using namespace std;
const double pi = 3.14;
bool is_valid_radius(double radius);
double circle_area(double radius);
double circle_circumference(double radius);
int main(){
    double radius;
    cin >> radius;
    cout << "Area: " << fixed << setprecision(2) << circle_area(radius) << endl;
    cout << "Circumference: " << fixed << setprecision(2) << circle_circumference(radius) << endl;
    return 0;
}
double circle_area(double radius){
    bool result = is_valid_radius(radius);
    if(!result){
        cout << "Infvalid radius" << endl;
        return -1;
    }
    return pi*radius*radius;
}
double circle_circumference(double radius){
    bool result = is_valid_radius(radius);
    if(!result){
        cout << "Infvalid radius" << endl;
        return -1;
    }
    return 2*pi*radius;
}
bool is_valid_radius(double radius){
    if(radius<=0)
        return false;
    return true;
}