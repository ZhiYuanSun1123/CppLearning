#pragma once
#include"model.hpp"
#include"request_result.hpp"
#include<iostream>
class WhisperBackend
    : public ModelBackend{
public:
    explicit WhisperBackend(
        const std::string& name
    );
    bool is_ready() const override;
    InferenceResult infer(
        const InferenceRequest& request
    ) const override;
    static int inference_count();
    ~WhisperBackend() = default;
private:
    inline static int inference_count_ = 0;
};