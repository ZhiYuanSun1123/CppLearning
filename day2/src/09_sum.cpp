#include<iostream>
using namespace std;
int main(){
    int n;
    int result=0;
    cin >> n;
    if(n<=0)
        cout << "Invalid input" << endl;
    else{
        for(int i=1; i<=n; i++)
            result += i;
        cout << "Sum: " << result << endl;
    }
    return 0;
}