#include<iostream>
class ModelConfig {
public:
    ModelConfig(
        const std::string& model_name,
        int batch_size
    ):model_name_(model_name),batch_size_(batch_size){
        if(this->batch_size_ <= 0)
            throw std::runtime_error("batch_size_ <= 0");
    }

    std::string get_model_name(){
        return this->model_name_;
    }
    std::string get_model_name() const{
        return this->model_name_;
    }
    int get_batch_size(){
        return this->batch_size_;
    }
    int get_batch_size() const{
        return this->batch_size_;
    }

private:
    std::string model_name_;
    int batch_size_;
};
int main(){
    const ModelConfig modelconfig("Qwen2.5_Omni",20);
    std::cout << "Model Name :" << modelconfig.get_model_name() << std::endl;
    std::cout << "Batch Size :" << modelconfig.get_batch_size() << std::endl;
    return 0;
}