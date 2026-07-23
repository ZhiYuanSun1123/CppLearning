#include "file_handle.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

void export_audio_result(
    const std::string& path,
    bool simulate_failure
) {
    FileHandle output(path, "w");

    output.write("audio_id,score\n");
    output.write("sample_001,0.95\n");
    output.flush();

    if (simulate_failure) {
        throw std::runtime_error("模拟音频推理结果处理中断");
    }

    output.write("sample_002,0.87\n");
}

int main() {
    try {
        export_audio_result("normal_result.csv", false);
        std::cout << "正常路径完成\n";
    } catch (const std::exception& error) {
        std::cerr << "正常路径意外失败: "
                  << error.what()
                  << '\n';
        return 1;
    }

    try {
        export_audio_result("exception_result.csv", true);
    } catch (const std::exception& error) {
        std::cerr << "异常路径已捕获: "
                  << error.what()
                  << '\n';
    }

    std::cout << "两条路径验证结束\n";
    return 0;
}