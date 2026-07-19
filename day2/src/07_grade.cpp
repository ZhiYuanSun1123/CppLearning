#include<iostream>
using namespace std;
int main(){
    int score;
    cin >> score;
    if(score>=90 && score<=100)
        cout << "Grade A" << endl;
    else if(score>=80 && score<=89)
        cout << "Grade B" << endl;
    else if(score>=70 && score<=79)
        cout << "Grade C" << endl;
    else if(score>=60 && score<=69)
        cout << "Grade D" << endl;
    else if(score>=0 && score <=59)
        cout << "Grade F" << endl;
    else
        cout << "Invalid score" << endl;
}