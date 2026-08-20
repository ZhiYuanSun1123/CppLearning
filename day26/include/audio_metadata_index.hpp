#include"audio_metadata.hpp"
#include<vector>
#include<map>
#include<optional>
using PrimaryIndex =
    std::unordered_map<std::string, AudioMetadata>;
struct AudioQuery {
    std::string label;
    int sample_rate;
    double minimum_duration;
    double maximum_duration;
    double minimum_quality;
};
class AudioMetadataIndex {
public:
    bool add(const AudioMetadata& metadata);

    bool update(
        const std::string& path,
        const AudioMetadata& replacement
    );

    bool remove(const std::string& path);

    std::optional<AudioMetadata> find_by_path(
        const std::string& path
    ) const;

    std::vector<AudioMetadata> find_by_label(
        const std::string& label
    ) const;

    std::vector<AudioMetadata> find_by_sample_rate(
        int sample_rate
    ) const;

    std::vector<AudioMetadata> query(/* 查询条件 */) const;

    std::size_t size() const noexcept;
    bool empty() const noexcept;
    bool validate_consistency() const;
private:
    PrimaryIndex datas;
    std::unordered_map<std::string,std::set<std::string>> paths_by_label_;
    std::map<int,std::set<std::string>> paths_by_sample_rate_;
    std::string normalize_path(std::string path);
};