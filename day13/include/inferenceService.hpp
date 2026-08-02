#pragma once
#include"model.hpp"
#include"request_result.hpp"
#include<iostream>
class InferenceService{
public:
    InferenceResult run(
        const ModelBackend& backend,
        const InferenceRequest& request
    ) const;
};