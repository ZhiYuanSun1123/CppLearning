#include<iostream>
class Model{
    public:
        Model(const std::string& name);

        const std::string& name() const;
        virtual void infer(
            const std::string& features
        ) const = 0;
        virtual ~Model() = default;
    private:
        std::string name_;
};
class WhisperModel : public Model {
public:
    explicit WhisperModel(
        const std::string& name
    );
    void infer(
        const std::string& features
    ) const override;
};
class QwenOmniModel : public Model {
public:
    explicit QwenOmniModel(
        const std::string& name
    );
    void infer(
        const std::string& features
    ) const override;
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
        int feature_size
    );

    void run(
        const Model& model,
        const std::string& path
    ) const;

private:
    AudioDecoder decoder_;
    FeatureExtractor extractor_;
};
Model::Model(const std::string& name):name_(name){
    if(name.size() == 0)
        throw std::runtime_error("name不能为空");
}
const std::string& Model::name() const{
    return name_;
}
WhisperModel::WhisperModel(const std::string& name):Model(name){}
void WhisperModel::infer(const std::string& features) const{
    std::cout << "WhisperModel Infering " << features << std::endl;
}
QwenOmniModel::QwenOmniModel(const std::string& name):Model(name){}
void QwenOmniModel::infer(const std::string& features) const{
    std::cout << "QwenOmniModel Infering " << features << std::endl;
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
    std::cout << "Extract "+std::to_string(feature_size_)+"-dim features from " << decoded_audio << std::endl;
    return "features";
}
AudioInferenceService::AudioInferenceService(const std::string& format,int feature_size):decoder_(format),extractor_(feature_size){}
void AudioInferenceService::run(const Model& model,const std::string& path) const{
    const std::string decoder = decoder_.decode(path);
    const std::string features = extractor_.extract(decoder);
    model.infer(features);
}
int main(){
    AudioInferenceService ais("None",100);
    WhisperModel whisper(
        "Whisper"
    );
    QwenOmniModel qwen(
        "Qwen2.5-Omni"
    );
    ais.run(whisper,"reasoning.wav");
    ais.run(qwen,"reasoning.wav");
    return 0;
}