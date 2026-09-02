#ifndef DAY36_AUDIO_SERVICE_STRUCTURED_LOGGER_HPP
#define DAY36_AUDIO_SERVICE_STRUCTURED_LOGGER_HPP

#include "audio_service/error_code.hpp"

#include <iosfwd>
#include <string>
#include <string_view>

namespace audio_service {

enum class LogLevel {
    debug,
    info,
    warning,
    error
};

std::string_view to_string(LogLevel level) noexcept;

struct LogRecord {
    std::string timestamp;
    LogLevel level{LogLevel::info};
    std::string event;
    std::string request_id;
    ErrorCode error_code{ErrorCode::ok};
    std::string message;
};

std::string escape_json(std::string_view text);
std::string to_json(const LogRecord& record);

class StructuredLogger {
public:
    explicit StructuredLogger(std::ostream& output) noexcept;
    void log(const LogRecord& record);

private:
    std::ostream& output_;
};

} // namespace audio_service

#endif
