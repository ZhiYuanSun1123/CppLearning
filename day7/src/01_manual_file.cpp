#include <cstdio>
#include <iostream>

int main() {
    std::FILE* file = std::fopen("manual.txt", "w");

    if (file == nullptr) {
        std::cerr << "打开文件失败\n";
        return 1;
    }

    std::fputs("manual resource management\n", file);

    if (std::fclose(file) != 0) {
        std::cerr << "关闭文件失败\n";
        return 1;
    }

    file = nullptr;
    return 0;
}