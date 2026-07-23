#include <cstdio>
#include <iostream>
#include <stdexcept>

void write_without_raii() {
    std::FILE* file = std::fopen("without_raii.txt", "w");

    if (file == nullptr) {
        throw std::runtime_error("打开文件失败");
    }

    std::fputs("data before exception\n", file);
    std::cout << "文件已经打开，准备抛出异常\n";

    throw std::runtime_error("模拟业务处理失败");

    std::fclose(file);
}

int main() {
    try {
        write_without_raii();
    } catch (const std::exception& error) {
        std::cerr << "捕获异常: " << error.what() << '\n';
    }

    return 0;
}