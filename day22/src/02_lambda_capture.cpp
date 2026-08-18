#include<iostream>
int main(){
    int threshold = 100;
    auto test = [threshold](){
        std::cout << "内部：" << threshold << std::endl;
    };
    threshold = 1000;
    test();
    auto test2 = [&threshold](){
        std::cout << "内部：" << threshold << std::endl;
    };
    threshold = 10000;
    test2();
    int count = 0;
    auto test3 = [count]()mutable{
        count ++;
        std::cout << "内部：" << count << std::endl;
    };
    test3();
    std::cout << "外部：" << count << std::endl;
}