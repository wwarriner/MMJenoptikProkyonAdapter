#pragma once

#ifndef PROKYON_PARAMETERS_H_
#define PROKYON_PARAMETERS_H_

#include "parameterif.h"

#include <cassert>
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
    template<typename T>
    NumericParameter<T> get_numeric_parameter(DijSDK_Handle handle, DijSDK_EParamId param_id, unsigned int length, DijSDK_EParamQuery query = DijSDK_EParamQueryCurrent);
    template<typename T>
    error_t set_numeric_parameter(DijSDK_Handle handle, DijSDK_EParamId param_id, const std::vector<T> &value);

    struct StringParameter {
        std::string value;
        error_t error;
    };
    StringParameter get_string_parameter(DijSDK_Handle handle, DijSDK_EParamId param_id, unsigned int length);

    // private
    template<typename T>
    error_t get_numeric_sdk_parameter(DijSDK_Handle handle, DijSDK_EParamId paramId, T *pValue, unsigned int num, DijSDK_EParamQuery query = DijSDK_EParamQueryCurrent);
    template<>
    error_t get_numeric_sdk_parameter(DijSDK_Handle handle, DijSDK_EParamId paramId, int *pValue, unsigned int num, DijSDK_EParamQuery query);
    template<>
    error_t get_numeric_sdk_parameter(DijSDK_Handle handle, DijSDK_EParamId paramId, double *pValue, unsigned int num, DijSDK_EParamQuery query);

    template<typename T>
    error_t set_numeric_sdk_parameter(DijSDK_Handle handle, DijSDK_EParamId paramId, T *pValue, unsigned int num);
    template<>
    error_t set_numeric_sdk_parameter(DijSDK_Handle handle, DijSDK_EParamId paramId, int *pValue, unsigned int num);
    template<>
    error_t set_numeric_sdk_parameter(DijSDK_Handle handle, DijSDK_EParamId paramId, double *pValue, unsigned int num);

    typename std::make_unsigned<int>::type to_unsigned(const int signed_value);
    std::vector<typename std::make_unsigned<int>::type> to_unsigned(const std::vector<int> &signed_value);
}

// template implementation
namespace Prokyon {
    template<typename T>
    NumericParameter<T> get_numeric_parameter(DijSDK_Handle handle, DijSDK_EParamId param_id, unsigned int length, DijSDK_EParamQuery query) {
        std::vector<T> value(length);
        auto result = get_numeric_sdk_parameter<T>(handle, param_id, value.data(), length, query);
        //assert(value.size() == length);
        return {value, result};
    }

    template<typename T>
    error_t set_numeric_parameter(DijSDK_Handle handle, DijSDK_EParamId paramId, const std::vector<T> &value) {
        std::vector<T> v(value);
        auto num = value.size();
        assert(num < (std::numeric_limits<unsigned int>::max)());
        return set_numeric_sdk_parameter<T>(handle, paramId, v.data(), static_cast<unsigned int>(num));
    };
}

#endif