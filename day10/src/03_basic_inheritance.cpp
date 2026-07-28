#include <iostream>
#include <stdexcept>
#include <string>

class Model {
public:
    Model(const std::string& name)
        : name_(name) {
        if (name_.empty()) {
            throw std::runtime_error(
                "模型名称不能为空"
            );
        }
    }

    const std::string& name() const {
        return name_;
    }

    void print_base_info() const {
        std::cout
            << "Model: "
            << name_
            << '\n';
    }

private:
    std::string name_;
};

class AudioModel : public Model {
public:
    AudioModel(
        const std::string& name,
        int sample_rate
    )
        : Model(name),
          sample_rate_(sample_rate) {
        if (sample_rate_ <= 0) {
            throw std::runtime_error(
                "采样率必须大于0"
            );
        }
    }

    int sample_rate() const {
        return sample_rate_;
    }

    void print_audio_info() const {
        print_base_info();

        std::cout
            << "Sample rate: "
            << sample_rate_
            << '\n';
    }

private:
    int sample_rate_;
};

int main() {
    AudioModel model(
        "audio-classifier",
        16000
    );

    model.print_audio_info();
    return 0;
}