#pragma once

#ifndef PROKYON_PARAMETERS_H_
#define PROKYON_PARAMETERS_H_

#include <parameterif.h>

#include <string>
#include <vector>
#include <utility>

using DijSDK_Handle = void *;
using error_t = int;

namespace Prokyon {
    template<typename T>
    struct NumericParameter {
        std::vector<T> value;
        error_t error;
    };

    struct StringParameter {
        std::string value;
        error_t error;
    };

    template<typename T>
    NumericParameter<T> get_numeric_parameter(DijSDK_Handle handle, DijSDK_EParamId param_id, const unsigned int length, DijSDK_EParamQuery query = DijSDK_EParamQueryCurrent);
    StringParameter get_string_parameter(DijSDK_Handle handle, DijSDK_EParamId param_id, const unsigned int length);
    template<typename T>
    error_t get_numeric_sdk_parameter(DijSDK_Handle handle, DijSDK_EParamId paramId, T *pValue, unsigned int num, DijSDK_EParamQuery query = DijSDK_EParamQueryCurrent);
    template<>
    error_t get_numeric_sdk_parameter(DijSDK_Handle handle, DijSDK_EParamId paramId, int *pValue, unsigned int num, DijSDK_EParamQuery query);
    template<>
    error_t get_numeric_sdk_parameter(DijSDK_Handle handle, DijSDK_EParamId paramId, double *pValue, unsigned int num, DijSDK_EParamQuery query);

    typename std::make_unsigned<int>::type to_unsigned(const int signed_value);

    std::vector<typename std::make_unsigned<int>::type> to_unsigned(const std::vector<int> &signed_value);
}

// template
namespace Prokyon {
    template<typename T>
    NumericParameter<T> get_numeric_parameter(DijSDK_Handle handle, DijSDK_EParamId param_id, const unsigned int length, DijSDK_EParamQuery query) {
        std::vector<int> value;
        value.reserve(length);
        auto result = get_numeric_sdk_parameter<T>(handle, param_id, value.data(), length, query);
        return {value, result};
    }
}

#endif