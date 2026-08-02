#include"model.hpp"
ModelBackend::ModelBackend(
    const std::string& name
)
    : name_(name){
    if(name_.empty())
        throw std::invalid_argument(
            "后端名称不能为空"
        );
}
const std::string& ModelBackend::name() const {
    return name_;
}