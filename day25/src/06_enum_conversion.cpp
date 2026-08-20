#include <cstdint>
enum class AudioFormat : std::uint8_t {
    wav = 1,
    flac = 2,
    mp3 = 3,
    unknown = 4
};
#include<iostream>
std::string to_string(AudioFormat format){
    switch(format){
        case AudioFormat::flac:
            return "flac";
        case AudioFormat::mp3:
            return "mp3";
        case AudioFormat::unknown:
            return "unknown";
        case AudioFormat::wav:
            return "wav";
    };
}
int main(){
    std::cout << static_cast<int>(AudioFormat::flac) << std::endl;
    std::cout << to_string(static_cast<AudioFormat>(1)) << std::endl;
}
