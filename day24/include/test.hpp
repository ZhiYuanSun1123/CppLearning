#include<iostream>
template <typename T>
class Box{
public:
    Box(T t);
    void print(){
        std::cout << t << std::endl;
    }
private:
    T t;
};
template <typename T>
Box<T>::Box(T t):t(t){}
// extern template class Box<int>;