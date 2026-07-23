#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>

class FileHandle {
public:
    FileHandle(const std::string& path, const char* mode)
        : file_(std::fopen(path.c_str(), mode)) {
        if (file_ == nullptr) {
            throw std::runtime_error("无法打开文件: " + path);
        }

        std::cout << "1. 构造：文件打开成功\n";
    }

    ~FileHandle() noexcept {
        if (file_ != nullptr) {
            std::fclose(file_);
            std::cout << "3. 析构：文件自动关闭\n";
        }
    }

    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    void write(const std::string& text) {
        if (std::fputs(text.c_str(), file_) == EOF) {
            throw std::runtime_error("文件写入失败");
        }
        throw std::runtime_error("模拟处理中途失败");
    }

private:
    std::FILE* file_;
};

void process_file() {
    FileHandle file("exception_path.txt", "w");
    file.write("data before exception\n");

    std::cout << "2. 即将抛出异常\n";


    std::cout << "这行不会执行\n";
}

int main() {
    try {
        process_file();
    } catch (const std::exception& error) {
        std::cout << "4. catch：捕获异常：" << error.what() << '\n';
    }

    std::cout << "5. 程序继续执行\n";
    return 0;
}