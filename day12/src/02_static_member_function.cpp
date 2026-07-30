#include<iostream>
class Model{
public:
    Model(const std::string& name)
        : name_(name){}
    void infer() const{
        ++request_count_;
        std::cout << "Infering" << std::endl;
    }
    static int request_count(){
        return request_count_;
    }
private:
    std::string name_;
    inline static int request_count_ = 0;
};
int main(){
    const Model ml("None");
    ml.infer();
    std::cout << Model::request_count() << std::endl;
    return 0;
}