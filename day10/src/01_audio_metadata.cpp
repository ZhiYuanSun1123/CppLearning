#include <iostream>
#include <stdexcept>
#include <string>

class AudioMetadata {
public:
    AudioMetadata(
        const std::string& path,
        int sample_rate,
        int channels,
        double duration_seconds
    );

    const std::string& path() const;
    int sample_rate() const;
    int channels() const;
    double duration_seconds() const;

    void set_duration(
        double duration_seconds
    );

    void print() const;

private:
    void validate() const;

    std::string path_;
    int sample_rate_;
    int channels_;
    double duration_seconds_;
};
void AudioMetadata::validate() const{
    if(path_.size()==0)
        throw std::runtime_error("path不能为空");
    if(sample_rate_<=0)
        throw std::runtime_error("sample_rate>0");
    if(channels_<=0)
        throw std::runtime_error("channels>0");
    if(duration_seconds_<0)
        throw std::runtime_error("duration>=0");  
}
void AudioMetadata::print() const{
    std::cout << "Path: " << path_ << std::endl;
    std::cout << "Sample Rate: " << sample_rate_ << std::endl;
    std::cout << "Channels: " << channels_ << std::endl;
    std::cout << "Duration Seconds: " << duration_seconds_ << std::endl; 
}
double AudioMetadata::duration_seconds() const{
    validate();
    return duration_seconds_;
}
int AudioMetadata::channels() const{
    validate();
    return channels_;
}
int AudioMetadata::sample_rate() const{
    validate();
    return sample_rate_;
}
const std::string& AudioMetadata::path() const{
    validate();
    return path_;
}
AudioMetadata::AudioMetadata(
    const std::string& path,
    int sample_rate,
    int channels,
    double duration_seconds
):path_(path),
  sample_rate_(sample_rate),
  channels_(channels),
  duration_seconds_(duration_seconds){
    validate();
}

int main(){
    try{
        // // path为空
        // AudioMetadata amd("",16000,3,100);
        // // Sample rate≦0
        // AudioMetadata amd("root",0,3,100);
        // // Channels≦0
        // AudioMetadata amd("root",16000,0,100);
        // Duration<0
        AudioMetadata amd("root",16000,3,-1);
    } catch(const std::exception& error){
        std::cerr
            << "Error: "
            << error.what()
            << '\n';
        return 1;
    }
}