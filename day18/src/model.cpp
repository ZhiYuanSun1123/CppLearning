#include"model.hpp"
ModelBackend::ModelBackend(
    const std::string& name
)
    : name_(name){
    if(name_.empty())
        throw std::invalid_argument(
            "后端名称不能为空"
        );
    ++alive_count_;
    ++created_count_;
    std::cout
        << "Construct backend: "
        << name_
        << std::endl;
}
ModelBackend::~ModelBackend(){
    --alive_count_;
    ++destroyed_count_;
    std::cout
        << "Destroy backend: "
        << name_
        << std::endl;
}
const std::string& ModelBackend::name() const {
    return name_;
}
int ModelBackend::alive_count() noexcept{
    return alive_count_;
}
int ModelBackend::created_count() noexcept{
    return created_count_;
}
int ModelBackend::destroyed_count() noexcept{
    return destroyed_count_;
}
