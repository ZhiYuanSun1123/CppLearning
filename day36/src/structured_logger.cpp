#include "audio_service/structured_logger.hpp"

#include <iomanip>
#include <ostream>
#include <sstream>

namespace audio_service {

std::string_view to_string(LogLevel level) noexcept {
    switch (level) {
    case LogLevel::debug: return "DEBUG";
    case LogLevel::info: return "INFO";
    case LogLevel::warning: return "WARNING";
    case LogLevel::error: return "ERROR";
    }
    return "UNKNOWN";
}

std::string escape_json(std::string_view text) {
    std::ostringstream escaped;
    for (const unsigned char character : text) {
        switch (character) {
        case '"': escaped << "\\\""; break;
        case '\\': escaped << "\\\\"; break;
        case '\n': escaped << "\\n"; break;
        case '\r': escaped << "\\r"; break;
        case '\t': escaped << "\\t"; break;
        default:
            if (character < 0x20U) {
                escaped << "\\u"
                        << std::hex << std::setw(4) << std::setfill('0')
                        << static_cast<int>(character)
                        << std::dec;
            } else {
                escaped << static_cast<char>(character);
            }
        }
    }
    return escaped.str();
}

std::string to_json(const LogRecord& record) {
    std::ostringstream json;
    json << '{'
         << "\"timestamp\":\"" << escape_json(record.timestamp) << "\","
         << "\"level\":\"" << to_string(record.level) << "\","
         << "\"event\":\"" << escape_json(record.event) << "\","
         << "\"request_id\":\"" << escape_json(record.request_id) << "\","
         << "\"error_code\":" << static_cast<int>(record.error_code) << ','
         << "\"error_name\":\"" << to_string(record.error_code) << "\","
         << "\"category\":\"" << error_category(record.error_code) << "\","
         << "\"message\":\"" << escape_json(record.message) << "\""
         << '}';
    return json.str();
}

StructuredLogger::StructuredLogger(std::ostream& output) noexcept
    : output_(output) {}

void StructuredLogger::log(const LogRecord& record) {
    output_ << to_json(record) << '\n';
}

} // namespace audio_service
