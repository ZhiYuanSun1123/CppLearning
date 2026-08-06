#pragma once
#include"request_result.hpp"
#include<iostream>
class ModelBackend{
    public:
        explicit ModelBackend(
            const std::string& name
        );
        const std::string& name() const;
        virtual bool is_ready() const = 0;
        virtual InferenceResult infer(
            const InferenceRequest& request
        ) const = 0;
        virtual ~ModelBackend() = default;
    private:
        std::string name_;
};