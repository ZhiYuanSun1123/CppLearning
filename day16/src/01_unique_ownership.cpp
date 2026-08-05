#include<iostream>
#include<memory>
#include<string>
#include<utility>

class Model{
public:
    explicit Model(
        const std::string& name
    ) : name_(name){
        std::cout << "Contstruct: " << name_ << std::endl;
    }
    ~Model(){
        std::cout << "Destory: " << name_ << std::endl;
    }
    void infer() const {
        std::cout << "Infer: " << name_ << std::endl;
    }
private:
    std::string name_;
};
int main(){
    auto first = std::make_unique<Model>("Qwen-Omni");
    first->infer();
    auto second = std::move(first);
    std::cout
        << "first is empyt: "
        << (first == nullptr)
        << std::endl;
    second->infer();
    return 0;
}
