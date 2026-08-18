#include<iostream>
template<typename T>
T add(const T& t1,const T& t2){
    return t1+t2;
}
int main(){
    std::cout << add(1,2) << std::endl;
    std::cout << add(1.0,2.0) << std::endl;
    return 0;
}