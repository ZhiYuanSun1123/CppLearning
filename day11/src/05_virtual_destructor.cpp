#include<iostream>
class Model {
public:
    virtual void infer() const = 0;

    virtual ~Model() {
        std::cout << "Destroy Model\n";
    }
};
class AudioModel : public Model {
public:
    void infer() const override {
    }

    ~AudioModel() override {
        std::cout << "Destroy AudioModel\n";
    }
};
int main() {
    Model* model = new AudioModel;
    delete model;
}