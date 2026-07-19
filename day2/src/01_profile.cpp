#include<iostream>
using namespace std;
int main(){
    string name;
    int age;
    float height;
    int onSchool;

    cout << "name" << endl;
    getline(cin,name);
    cout << "age" << endl;
    cin >> age;
    cout << "height" << endl;
    cin >> height;
    cout << "onSchool" << endl;
    cin >> onSchool;

    cout << "Name: " << name << endl;
    cout << "Age: " << age << endl;
    cout << "Height: " << height << "m" << endl;
    if(onSchool)
        cout << "Student: true"<< endl;
    else
        cout << "Student: false"<< endl;

    return 0;
}