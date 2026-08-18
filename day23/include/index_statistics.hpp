#include<cstddef>
struct IndexStatistic{
    std::size_t record_count;
    double total_duration_seconds;
    double average_duration_seconds;
    double minmum_duration_seconds;
    double maximum_duration_seconds;
    std::size_t label_counts;
    std::size_t sample_rate_counts;
};