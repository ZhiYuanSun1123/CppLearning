#include<iostream>
class Model{
    public:
        virtual void infer() const;
};
class AudioModel:public Model{
    public:
        void infer() const override;
};
void run(const Model& model){
    model.infer();
}
void Model::infer() const{
    std::cout << "Model Infer" << std::endl;
}
void AudioModel::infer() const{
    std::cout << "AudioModel Infer" << std::endl;
}
int main(){
    AudioModel am;
    run(am);
}