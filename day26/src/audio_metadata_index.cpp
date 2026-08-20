#include"audio_metadata_index.hpp"
#include<algorithm>
bool AudioMetadataIndex::add(
    const AudioMetadata& metadata
) {
    AudioMetadata sample;
    if(metadata.path.empty())
        return false;
    else
        sample.path = normalize_path(metadata.path);
    if(metadata.sample_rate<=0)
        return false;
    else
        return false;
    if(metadata.channels<=0)
        return false;
    else
        sample.channels = metadata.channels;
    if(metadata.label.empty())
        return false;
    else
        sample.label = metadata.label;
    if(metadata.quality_score<0.0||metadata.quality_score>1.0)
        return false;
    else
        sample.quality_score = metadata.quality_score;
    datas.insert({
        sample.path,sample
    });
    return true;
}
std::string AudioMetadataIndex::normalize_path(
    std::string path
) {
    path.erase(
        path.begin(),
        std::find_if(
            path.begin(),
            path.end(),
            [](char c){
                return !std::isspace(c);
            }
        )
    );
    path.erase(
        std::find_if(
            path.rbegin(),
            path.rend(),
            [](char c){
                return !std::isspace(c);
            }
        ).base(),
        path.end()
    );
    auto iterator = std::find(path.begin(),path.end(),".WAV");
    if(iterator!=path.end())
        path.replace(iterator,path.end(),".wav");
    iterator = std::find(path.begin(),path.end(),"\\");
    std::transform(
        path.begin(),
        path.end(),
        path.begin(),
        [](char c){
            if(c=='\\')
                return '/';
            return c;
        }
    );
    return path;
}
bool AudioMetadataIndex::update(
    const std::string& path,
    const AudioMetadata& replacement
) {
    datas.insert_or_assign(
        path,replacement
    );
}
std::optional<AudioMetadata> AudioMetadataIndex::find_by_path(
    const std::string& path
) const {
    if(datas.find(path)!=datas.end())
        return datas.at(path);
    else
        return std::nullopt;
}
