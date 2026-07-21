#include <iostream>
using namespace std;
int main() {
    int outer = 10;
    std::cout << "outer value: " << outer << '\n';
    std::cout << "outer address: " << &outer << '\n';

    // {
    //     int inner = 20;
    //     std::cout << "inner value: " << inner << '\n';
    //     std::cout << "inner address: " << &inner << '\n';
    // }

    int inner = 20;
    std::cout << "inner value: " << inner << '\n';
    std::cout << "inner address: " << &inner << '\n';

    // 尝试在这里访问inner，观察编译器错误，然后恢复为可编译状态
    cout << inner << endl;
    return 0;
}