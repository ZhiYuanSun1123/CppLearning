#include<string>
struct AudioMetadata{
    std::string path;
    int sample_rate;
    int channels;
    double duration_seconds;
    std::string label;
    double quality_score;
};
