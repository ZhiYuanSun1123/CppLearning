#include<iostream>
int add(int n1,int n2){
    return n1 + n2;
}
template<typename T>
T add(const T& t1, const T& t2){
    return t1 + t2;
}
int add(int* n1, int* n2){
    return *n1 + *n2;
}
int main(){
    int num_1 = 1;
    int num_2 = 2;
    int* n1 = &num_1;
    int* n2 = &num_2;
    add(num_1,num_2);
    add(n1,n2);
    add(1.0,2.0);
}