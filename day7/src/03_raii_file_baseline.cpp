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

        std::cout << "FileHandle: 文件打开成功\n";
    }

    ~FileHandle() noexcept {
        if (file_ != nullptr) {
            std::fclose(file_);
            std::cout << "FileHandle: 文件已经关闭\n";
        }
    }

    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    void write(const std::string& text) {
        if (std::fputs(text.c_str(), file_) == EOF) {
            throw std::runtime_error("文件写入失败");
        }
    }

private:
    std::FILE* file_;
};

int main() {
    try {
        FileHandle file("raii_baseline.txt", "w");
        file.write("RAII baseline\n");
    } catch (const std::exception& error) {
        std::cerr << "错误: " << error.what() << '\n';
        return 1;
    }

    std::cout << "main 正常结束\n";
    return 0;
}