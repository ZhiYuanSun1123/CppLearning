#include<iostream>
class AudioClip {
public:
    AudioClip(
        const std::string& path,
        int sample_rate
    ):path_(path),sample_rate_(sample_rate){};

    AudioClip():path_("unknown"),sample_rate_(48000){};

    void print() const;

private:
    std::string path_;
    int sample_rate_;
};
void AudioClip::print() const{
    std::cout << "Path = " << this->path_ << std::endl;
    std::cout << "Sample Rate = " << this->sample_rate_ << std::endl; 
}
int main() {
    AudioClip first;
    AudioClip second(
        "music.wav",
        48000
    );

    first.print();
    second.print();
    return 0;
}