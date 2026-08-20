template<typename T>
auto add(T t1,T t2)->decltype(t1+t2){
    return t1+t2;
}
#include<iostream>
int main(){
    auto result = add<int>(1,2.0);
    static_assert(
        std::is_same_v<decltype(result),int>
    );
}