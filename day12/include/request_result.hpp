#pragma once
#include<iostream>
class InferenceRequest{
    public:
        InferenceRequest(
            const std::string& audio_path,
            const std::string& question
        );
        const std::string& audio_path() const;
        const std::string& question() const;
    private:
        std::string audio_path_;
        std::string question_;
};
class InferenceResult{
    public:
        InferenceResult(
            const std::string& backend_name,
            const std::string& ouput
        );
        const std::string& backend_name() const;
        const std::string& output() const;
        void print() const;
    private:
        std::string backend_name_;
        std::string output_;
};
