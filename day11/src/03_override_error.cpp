#include<iostream>
class Model {
public:
    virtual void infer(
        const std::string& input
    ) const {
    }
};
class AudioModel : public Model {
public:
    void infer(
        const std::string& input
    )const override {
    }
};
int main(){
    
}