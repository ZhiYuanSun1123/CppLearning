#include <iostream>

class Model {
public:
    virtual void infer() const {
        std::cout << "Model::infer\n";
    }
    virtual ~Model(){
        std::cout << "Destroy Model" << std::endl;
    }
};
class AudioModel : public Model {
public:
    AudioModel():id(new int(1)){}
    void infer() const override{
        std::cout << "AudioModel::infer\n";
    }
    ~AudioModel(){
        delete id;
        std::cout << "Destroy AudioModel" << std::endl;
    } 
private:
    int* id;
};
int main() {
    Model* model = new AudioModel;
    delete model;
}