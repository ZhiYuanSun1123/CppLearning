#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>

class FileHandle {
public:
    FileHandle(
        const std::string& path,
        const char* mode
    ) {
        file_ = std::fopen(
            path.c_str(),
            mode
        );

        if (file_ == nullptr) {
            throw std::runtime_error(
                "文件打开失败"
            );
        }

        std::cout << "文件打开\n";
    }

    ~FileHandle() {
        if (file_ != nullptr) {
            std::fclose(file_);
            std::cout << "文件关闭\n";
        }
    }

    void write(const std::string& text) {
        std::fputs(text.c_str(), file_);
    }

private:
    std::FILE* file_ = nullptr;
};

void process() {
    FileHandle file("raii_result.txt", "w");
    file.write("data before exception\n");

    std::cout << "抛出异常\n";
    // throw std::runtime_error("模拟失败");
}

int main() {
    try {
        process();
    } catch (const std::exception& error) {
        std::cerr << "捕获异常: "
                  << error.what()
                  << '\n';
    }

    return 0;
}