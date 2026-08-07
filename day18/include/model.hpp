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
        virtual ~ModelBackend();

        static int alive_count() noexcept;
        static int created_count() noexcept;
        static int destroyed_count() noexcept;
    private:
        std::string name_;
        inline static int alive_count_ = 0;
        inline static int created_count_ = 0;
        inline static int destroyed_count_ = 0;
};