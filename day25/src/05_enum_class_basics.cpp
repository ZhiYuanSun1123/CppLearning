enum class AudioFormat{
    wav = 1,
    flac = 2,
    mp3 = 3,
    unknown = 4
};
enum class ProcessingState{
    pending = 1,
    running = 2,
    completed = 3,
    failed = 4
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
    AudioFormat format = AudioFormat::wav;
    int value = static_cast<int>(format);
    std::cout << value << std::endl;
    auto format_result = static_cast<AudioFormat>(1);
    std::cout << to_string(format_result) << std::endl;
}