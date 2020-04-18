#include "Parameters.h"

#include "dijsdk.h"
#include "parameterif.h"

#include <algorithm>

namespace Prokyon {
    StringParameter get_string_parameter(DijSDK_Handle handle, DijSDK_EParamId param_id, unsigned int length) {
        std::vector<char> value;
        value.reserve(length);
        auto result = DijSDK_GetStringParameter(handle, param_id, value.data(), length);
        return {std::string(value.cbegin(), value.cend()), result};
    }

    template<>
    error_t get_numeric_sdk_parameter(DijSDK_Handle handle, DijSDK_EParamId paramId, int *pValue, unsigned int num, DijSDK_EParamQuery query) {
        return DijSDK_GetIntParameter(handle, paramId, pValue, num, query);
    }

    template<>
    error_t get_numeric_sdk_parameter(DijSDK_Handle handle, DijSDK_EParamId paramId, double *pValue, unsigned int num, DijSDK_EParamQuery query) {
        return DijSDK_GetDoubleParameter(handle, paramId, pValue, num, query);
    }

    template<>
    error_t set_numeric_sdk_parameter(DijSDK_Handle handle, DijSDK_EParamId paramId, int *pValue, unsigned int num) {
        return DijSDK_SetIntParameterArray(handle, paramId, pValue, num);
    }

    template<>
    error_t set_numeric_sdk_parameter(DijSDK_Handle handle, DijSDK_EParamId paramId, double *pValue, unsigned int num) {
        return DijSDK_SetDoubleParameterArray(handle, paramId, pValue, num);
    }

    typename std::make_unsigned<int>::type to_unsigned(const int signed_value) {
        return static_cast<typename std::make_unsigned<int>::type>(signed_value);
    }

    std::vector<typename std::make_unsigned<int>::type> to_unsigned(const std::vector<int> &signed_value) {
        std::vector<typename std::make_unsigned<int>::type> unsigned_value;
        auto transform_function = [](const int v) -> typename std::make_unsigned<int>::type {return to_unsigned(v); };
        std::transform(signed_value.cbegin(), signed_value.cend(), unsigned_value.begin(), transform_function);
        return unsigned_value;
    }
}