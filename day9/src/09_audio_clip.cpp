#include <iostream>
#include <stdexcept>
#include <string>

class AudioClip {
public:
    AudioClip();

    AudioClip(
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

    void print_summary() const;

    ~AudioClip();

private:
    std::string path_;
    int sample_rate_;
    int channels_;
    double duration_seconds_;
};
AudioClip::AudioClip():
    path_("unknown"),
    sample_rate_(16000),
    channels_(1),
    duration_seconds_(0.0){}
AudioClip::AudioClip(
    const std::string& path,
    int sample_rate,
    int channels,
    double duration_seconds
):
    path_(path),
    sample_rate_(sample_rate),
    channels_(channels),
    duration_seconds_(duration_seconds){
    if(path.size()==0)
        throw std::runtime_error("path为空");
    if(sample_rate<=0)
        throw std::runtime_error("sample_rate必须大于0");
    if(channels<=0)
        throw std::runtime_error("channels必须大于0");
    if(duration_seconds<0)
        throw std::runtime_error("duration不能小于0");
}
const std::string& AudioClip::path() const{
    return this->path_;
}
int AudioClip::sample_rate() const{
    return this->sample_rate_;
}
int AudioClip::channels() const{
    return this->channels_;
}
double AudioClip::duration_seconds() const{
    return this->duration_seconds_;
}
void AudioClip::set_duration(double duration_seconds){
    if(duration_seconds<0)
        throw std::runtime_error("负数输入");
    this->duration_seconds_ = duration_seconds;
}
AudioClip::~AudioClip(){
    std::cout << "Destroy AudioClip: " << this->path_ << std::endl;
}
void AudioClip::print_summary() const{
    std::cout << "Path: " << this->path_ << std::endl;
    std::cout << "Sample Rate: " << this->sample_rate_ << std::endl;
    std::cout << "Channels: " << this->channels_ << std::endl;
    std::cout << "Duration Seconds: " << this->duration_seconds_ << std::endl;
}
int main() {
    try {
        AudioClip first;

        AudioClip second(
            "speech.wav",
            16000,
            1,
            3.5
        );

        // AudioClip invalid_path(
        //     "",
        //     16000,
        //     1,
        //     1.0
        // );
        // AudioClip invalid_rate(
        //     "speech.wav",
        //     0,
        //     1,
        //     1.0
        // );
        // AudioClip invalid_channels(
        //     "speech.wav",
        //     16000,
        //     0,
        //     1.0
        // );
        // AudioClip invalid_duration(
        //     "speech.wav",
        //     16000,
        //     1,
        //     -1.0
        // );
        first.print_summary();
        second.print_summary();

        second.set_duration(4.0);

        const AudioClip fixed(
            "music.wav",
            48000,
            2,
            120.0
        );

        std::cout
            << "Fixed sample rate: "
            << fixed.sample_rate()
            << '\n';

        return 0;
    } catch (
        const std::exception& error
    ) {
        std::cerr
            << "Error: "
            << error.what()
            << '\n';
        return 1;
    }
}