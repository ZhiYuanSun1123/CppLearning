#include<iostream>
#include<memory>

class Model{
public:
    ~Model(){
        std::cout << "Destory Model" << std::endl;
    }
    void infer() const {
        std::cout << "Infer" << std::endl;
    }
};
int main(){
    std::weak_ptr<Model> observer;
    {
        auto owner = std::make_shared<Model>();
        observer = owner;
        if(auto model = observer.lock()){
            model->infer();
        }
    }

    if(auto model = observer.lock()){
        model->infer();
    } else {
        std::cout << "Model expired" << std::endl;
    }
    return 0;
}
