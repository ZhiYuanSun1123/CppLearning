constexpr double milliseconds_to_samples(
    int seconds
) {
    return 1/seconds*1000;
}
constexpr double samples_to_milliseconds(
    int sample_rate
) {
    return 1000/sample_rate;
}
constexpr double clamp_sample_rate(
    double min,double max, double sample_rate
) {
    if(sample_rate < min)
        return min;
    if(sample_rate > max)
        return max;
    return sample_rate;
}
constexpr bool is_supported_sample_rate(
    double sample_rate
) {
    if(sample_rate <= 0)
        return false;
    return true;
}
constexpr double bytes_for_pcm(
    int channels,
    double sample_rate,
    int seconds,
    int bytes
) {
    return channels*sample_rate*seconds*bytes;
}
