#include <iostream>
#include <string>

class AudioClip {
public:
    void set_path(const std::string& path) {
        // TODO
        this->path_ = path;
    }

    void set_sample_rate(int sample_rate) {
        // TODO：只接受大于0的采样率
        if(sample_rate<=0){
            std::cout << "Invalid Rate" << std::endl;
            return;
        }
        this->sample_rate_ = sample_rate;
    }

    const std::string& path() const {
        // TODO
        return this->path_;
    }

    int sample_rate() const {
        // TODO
        return this->sample_rate_;
    }

    void print() const {
        // TODO
        std::cout << "Path: " << this->path_ << std::endl;
        std::cout << "Sample rate: " << this->sample_rate_ << std::endl;
    }

private:
    std::string path_;
    int sample_rate_ = 0;
};

int main() {
    AudioClip clip;

    clip.set_path("speech.wav");
    clip.set_sample_rate(16000);
    clip.print();

    return 0;
}