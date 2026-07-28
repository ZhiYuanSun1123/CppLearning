#include<iostream>
class Model{
    public:
        Model(const std::string& name);

        const std::string& name() const;
        void print_model_info() const;
    
    private:
        std::string name_;
};
class AudioModel:public Model{
    public:
        AudioModel(
            const std::string name,
            int sample_rate
        );

        int sample_rate() const;
        
        void infer(
            const std::string& features
        ) const;
    private:
        int sample_rate_;
};
class AudioDecoder {
public:
    AudioDecoder(
        const std::string& format
    );

    std::string decode(
        const std::string& path
    ) const;

private:
    std::string format_;
};
class FeatureExtractor {
public:
    FeatureExtractor(
        int feature_size
    );

    std::string extract(
        const std::string& decoded_audio
    ) const;

private:
    int feature_size_;
};
class AudioInferenceService {
public:
    AudioInferenceService(
        const std::string& format,
        int feature_size,
        const std::string& model_name,
        int sample_rate
    );

    void run(
        const std::string& path
    ) const;

private:
    AudioDecoder decoder_;
    FeatureExtractor extractor_;
    AudioModel model_;
};
Model::Model(const std::string& name):name_(name){
    if(name.size() == 0)
        throw std::runtime_error("name不能为空");
}
const std::string& Model::name() const{
    return name_;
}
void Model::print_model_info() const{
    std::cout << "Model Name: " << name_ << std::endl;
}
AudioModel::AudioModel(
    const std::string name,
    int sample_rate
):Model(name),
  sample_rate_(sample_rate){
    if(sample_rate<=0)
        throw std::runtime_error("sample rate ≦ 0");
}
int AudioModel::sample_rate() const{
    return sample_rate_;
}
void AudioModel::infer(const std::string& features) const{
    std::cout << "Run audio-model with decoded " + features << std::endl;
}
AudioDecoder::AudioDecoder(const std::string& format):format_(format){
    if(format.size() == 0)
     throw std::runtime_error("format不能为空");
}
std::string AudioDecoder::decode(const std::string& path) const{
    std::cout << "Decode "+ path + " as " + format_ << std::endl;
    return "audio-model";
}
FeatureExtractor::FeatureExtractor(int feature_size):feature_size_(feature_size){
    if(feature_size<=0)
        throw std::runtime_error("feature size ≦ 0");
}
std::string FeatureExtractor::extract(const std::string& decoded_audio) const{
    std::cout << "Extract "+std::to_string(feature_size_)+"-dim features" << std::endl;
    return "features";
}
AudioInferenceService::AudioInferenceService(const std::string& format,int feature_size,const std::string& model_name,int sample_rate):decoder_(format),extractor_(feature_size),model_(model_name,sample_rate){}
void AudioInferenceService::run(const std::string& path) const{
    const std::string decoder = decoder_.decode(path);
    const std::string features = extractor_.extract(decoder);
    model_.infer(features);
}
int main(){
    AudioInferenceService ais("None",100,"Qwen",16000);
    ais.run("root");
    return 0;
}