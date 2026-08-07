#pragma once
#include"model.hpp"
#include"request_result.hpp"
#include<iostream>
#include<memory>
class InferenceService{
public:
    InferenceService(std::unique_ptr<ModelBackend> backend);
    InferenceResult run(
        const InferenceRequest& request
    ) const;
    void replace_backend(std::unique_ptr<ModelBackend> backend);
private:
    std::unique_ptr<ModelBackend> backend_;
};