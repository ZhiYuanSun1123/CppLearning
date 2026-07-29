#include<iostream>
class Model{
    public:
        Model(const std::string& name);
        virtual void infer(const std::string& input) const = 0;
        virtual ~Model() = default;
    private:
        std::string name_;
};
class AudioModel:public Model{
    public:
        AudioModel(const std::string& name);
        void infer(const std::string& input) const override;
        ~AudioModel() = default;
};
class TextModel:public Model{
    public:
        TextModel(const std::string& name);
        void infer(const std::string& input) const override;
        ~TextModel() = default;
};
void run_model(const Model& model,const std::string& input);
void run_model(
    const Model& model,
    const std::string& input
){
    model.infer(input);
}
void TextModel::infer(const std::string& input) const{
    std::cout << "TextModel Infering " << input << std::endl;
}
TextModel::TextModel(const std::string& name):Model(name){}
void AudioModel::infer(const std::string& input) const{
    std::cout << "AudioModel Infering " << input << std::endl;
    
}
AudioModel::AudioModel(const std::string& name):Model(name){}
Model::Model(const std::string& name):name_(name){
    if(name.size()==0)
        throw std::runtime_error("模型名称不能为空");
}
int main(){
    AudioModel am("QwenOmni");
    TextModel tm("Qwen");
    run_model(am,"input");
    run_model(tm,"input");
}