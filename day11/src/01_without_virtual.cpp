#include<iostream>
class Model{
    public:
        void infer() const;
};
class AudioModel:public Model{
    public:
        void infer() const;
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