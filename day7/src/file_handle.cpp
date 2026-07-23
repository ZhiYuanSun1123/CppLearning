#include"file_handle.hpp"
#include<stdexcept>
FileHandle::FileHandle(const std::string& path,const char*mode)
    :file_(std::fopen(path.c_str(),mode)){
    if(file_==nullptr){
        throw std::runtime_error("无法打开文件"+path);
    }
}
FileHandle::~FileHandle() noexcept{
    if(file_!=nullptr){
        std::fclose(file_);
    }
}
void FileHandle::write(const std::string& text){
    if(std::fputs(text.c_str(),file_)==EOF){
        throw std::runtime_error("文件写入失败");
    }
}
void FileHandle::flush(){
    if(std::fflush(file_)!=0){
        throw std::runtime_error("刷新文件失败");
    }
}
std::FILE* FileHandle::get() noexcept{
    return file_;
}
const std::FILE* FileHandle::get() const noexcept{
    return file_;
}