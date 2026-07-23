#include "file_handle.hpp"

#include <exception>
#include <iostream>

int main() {
    try {
        FileHandle file(
            "/this/directory/should/not/exist/result.txt",
            "w"
        );

        std::cout << "不应该执行到这里\n";
    } catch (const std::exception& error) {
        std::cerr << "预期内的打开失败: "
                  << error.what()
                  << '\n';
    }

    return 0;
}