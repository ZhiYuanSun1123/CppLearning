#include <iostream>
#include <string>

class ModelConfig {
public:
    ModelConfig(
        const std::string& name,
        int batch_size
    )
        : name_(name),
          batch_size_(batch_size) {
    }

    const std::string& name() const {
        return name_;
    }

    int batch_size() const {
        return batch_size_;
    }

    void set_batch_size(int batch_size) {
        if (batch_size > 0) {
            batch_size_ = batch_size;
        }
    }

private:
    std::string name_;
    int batch_size_;
};

int main() {
    const ModelConfig config(
        "audio-model",
        8);
    std::cout << config.name() << '\n';
    std::cout << config.batch_size() << '\n';

    return 0;
}