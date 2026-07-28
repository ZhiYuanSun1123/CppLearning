#include <iostream>
#include <string>

class AudioDecoder {
public:
    AudioDecoder(
        const std::string& format
    );

    void decode(
        const std::string& path
    ) const;

private:
    std::string format_;
};

class AudioPipeline {
public:
    AudioPipeline(
        const std::string& format
    );

    void run(
        const std::string& path
    ) const;

private:
    AudioDecoder decoder_;
};
void AudioPipeline::run(const std::string& path) const{
    decoder_.decode(path);
}
AudioPipeline::AudioPipeline(const std::string& format):decoder_(format){}
void AudioDecoder::decode(const std::string& path) const{
    std::cout << "Load Decoder: " << path << std::endl;
}
AudioDecoder::AudioDecoder(const std::string& format):format_(format){
    if(format_.size()==0)
        throw std::runtime_error("format不能为空");
}
int main(){
    AudioPipeline apl("None");
    apl.run("root");
}