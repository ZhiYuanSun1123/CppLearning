#include<iostream>
class ModelCounter{
public:
    ModelCounter();
    static int created_count();
private:
    inline static int created_count_ = 0;
};
ModelCounter::ModelCounter(){
    ++created_count_;
}
int ModelCounter::created_count(){
    return created_count_;
}
int main(){
    ModelCounter mc1;
    ModelCounter mc2;
    ModelCounter mc3;
    std::cout << ModelCounter::created_count() << std::endl;
    return 0;
}