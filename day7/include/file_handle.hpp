#ifndef FILE_HANDLE_HPP
#define FILE_HANDLE_HPP

#include <cstdio>
#include <string>

class FileHandle {
public:
    FileHandle(const std::string& path, const char* mode);
    ~FileHandle() noexcept;

    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    void write(const std::string& text);
    void flush();

    std::FILE* get() noexcept;
    const std::FILE* get() const noexcept;

private:
    std::FILE* file_;
};

#endif
