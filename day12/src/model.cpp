#include"model.hpp"
ModelBackend::ModelBackend(
    const std::string& name
)
    : name_(name){
    if(name.empty())
        throw std::runtime_error(
            "后缀名称不能为空"
        );
}
const std::string& ModelBackend::name() const {
    return name_;
}