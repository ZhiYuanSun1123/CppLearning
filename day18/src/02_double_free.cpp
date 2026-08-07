#include<memory>
int main(){
    auto value = std::make_unique<int>(42);
    int* raw = value.get();
    delete raw;
    return 0;
}