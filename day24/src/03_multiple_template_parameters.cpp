#include<iostream>
template<typename T,typename P>
T convert_value(const P& value){
    return static_cast<T>(value);
}
int main(){
    std::cout << convert_value<double>(10) << std::endl;
}
