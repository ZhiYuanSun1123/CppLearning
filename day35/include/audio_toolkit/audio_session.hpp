#ifndef DAY35_AUDIO_TOOLKIT_AUDIO_SESSION_HPP
#define DAY35_AUDIO_TOOLKIT_AUDIO_SESSION_HPP

#include <memory>
#include <string>

namespace audio_toolkit {

// 测试可观察状态：模拟“资源已获取/已释放”，不接触真实硬件。
struct ResourceState {
    int active_count{0};
    int acquire_count{0};
    int release_count{0};
};

class TrackedResource {
public:
    explicit TrackedResource(std::shared_ptr<ResourceState> state);
    ~TrackedResource() noexcept;

    TrackedResource(const TrackedResource&) = delete;
    TrackedResource& operator=(const TrackedResource&) = delete;

private:
    std::shared_ptr<ResourceState> state_;
};

class AudioSession {
public:
    AudioSession(std::shared_ptr<ResourceState> state,
                 std::string model_name,
                 bool fail_after_acquire = false);

    const std::string& model_name() const noexcept;

private:
    // 声明顺序决定构造和析构顺序：先获取资源，再初始化名称。
    TrackedResource resource_;
    std::string model_name_;
};

} // namespace audio_toolkit

#endif
