#include<iostream>
class Model {
public:
    virtual std::string type() const {
        return "Model";
    }

    virtual ~Model() = default;
};
class AudioModel : public Model {
public:
    std::string type() const override {
        return "AudioModel";
    }
};
void print_by_value(Model model);
void print_by_reference(const Model& model);
void print_by_value(Model model){
    std::cout << model.type() << std::endl;
}
void print_by_reference(const Model& model){
    std::cout << model.type() << std::endl;
}
int main(){
    AudioModel am;
    print_by_value(am);
    print_by_reference(am);
    return 0;
}
