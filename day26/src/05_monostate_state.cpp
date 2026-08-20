#include<iostream>
#include"audio_metadata.hpp"
enum class ParseErrorCode{
    error = 1
};
struct ParseError{
    ParseErrorCode code;
    std::string field;
    std::string message;
    std::size_t position;
};
int main(){
   std::variant<
        std::monostate,
        AudioMetadata,
        ParseError
    > result;
    result = ParseError{};
    std::visit(
        [](const auto& value){
            if constexpr(std::is_same_v<std::decay_t<decltype(value)>,std::monostate>)
                std::cout << "MonoState" << std::endl;
            if constexpr(std::is_same_v<std::decay_t<decltype(value)>,AudioMetadata>)
                std::cout << "AudioMetaData" << std::endl;
            if constexpr(std::is_same_v<std::decay_t<decltype(value)>,ParseError>)
                std::cout << "ParseError" << std::endl;
        },
        result
    );
}