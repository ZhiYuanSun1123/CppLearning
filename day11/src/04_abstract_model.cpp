#include<iostream>
class Model {
public:
    explicit Model(
        const std::string& name
    );

    const std::string& name() const;

    virtual void infer(
        const std::string& input
    ) const = 0;

    virtual ~Model() = default;

private:
    std::string name_;
};
const std::string& Model::name() const{
    return name_;
}
Model::Model(const std::string& name):name_(name){
    if(name.size()==0)
        throw std::runtime_error("模型名称不能为空");
}
class AudioModel : public Model{
    public:
        AudioModel(const std::string& name,int sample_rate);
        void infer(const std::string& input) const override;
        ~AudioModel()=default;
    private:
        int sample_rate_;
};
void AudioModel::infer(const std::string& input) const{
    std::cout << "AudioModel Infer " << input << std::endl;
}
AudioModel::AudioModel(const std::string& name,int sample_rate):Model(name),sample_rate_(sample_rate){
    if(sample_rate<=0)
        throw std::runtime_error("采样率不能≦0");
}
int main(){
    
}