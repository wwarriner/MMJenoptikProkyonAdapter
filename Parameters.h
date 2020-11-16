#pragma once

#ifndef PROKYON_PARAMETERS_H_
#define PROKYON_PARAMETERS_H_

#include "dijsdk.h"
#include "parameterif.h"

#include <cassert>
#include <string>
#include <vector>
#include <utility>
#include <set>
#include <map>

using DijSDK_Handle = void *;
using error_t = int;

namespace Prokyon {
    class StringProperty {
    public:
        StringProperty(DijSDK_Handle handle, DijSDK_EParamId id);
        std::string get() const;

    private:
        DijSDK_Handle m_handle;
        DijSDK_EParamId m_id;
    };

    template<typename T>
    class NumericProperty {
    public:
        NumericProperty(DijSDK_Handle handle, DijSDK_EParamId id);

        unsigned dimension() const; // throws
        bool writeable() const; // throws

        bool is_discrete() const; // throws
        std::vector<T> range() const; // is_discrete == false; size 2; throws
        bool allowed(unsigned index) const;

        std::vector<T> get() const; // throws
        T get(unsigned index) const; // throws

        void set(const std::vector<T> &value); // throws
        void set(const T &value, unsigned index); // throws

        std::string to_string() const;

    private:
        DijSDK_Handle m_handle;
        DijSDK_EParamId m_id;
    };

    template<typename T>
    class MappedNumericScalarProperty {
    public:
        MappedNumericScalarProperty(NumericProperty<T> prop, std::map<std::string, T> forward);

        bool writeable() const;

        std::vector<std::string> &range();
        const std::vector<std::string> &range() const;

        std::string get() const;
        void set(const std::string value);

    private:
        NumericProperty<T> m_property;
        std::map<std::string, T> m_forward;
        std::map<T, std::string> m_reverse;
        std::vector<std::string> m_range;
    };

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
    NumericProperty<T>::NumericProperty(DijSDK_Handle handle, DijSDK_EParamId id) :
        m_handle{handle},
        m_id{id}
    {}

    template<typename T>
    unsigned NumericProperty<T>::dimension() const {
        unsigned dimension = 0;
        auto result = DijSDK_GetParameterSpec(m_handle, m_id, nullptr, nullptr, &dimension);
        if (result) {
            assert(false);
            // todo throw
        }
        return dimension;
    }

    template<typename T>
    bool NumericProperty<T>::writeable() const {
        DijSDK_EParamAccess access = static_cast<DijSDK_EParamAccess>(0);
        auto result = DijSDK_GetParameterSpec(m_handle, m_id, nullptr, &access);
        if (result) {
            assert(false);
            // todo throw
        }
        return (access == DijSDK_EParamAccessReadWrite) || (access == DijSDK_EParamAccessWriteOnly);
    }

    template<typename T>
    bool NumericProperty<T>::is_discrete() const {
        DijSDK_EParamValueType value_type = static_cast<DijSDK_EParamValueType>(0);
        auto result = DijSDK_GetParameterSpec(m_handle, m_id, nullptr, nullptr, nullptr, &value_type);
        if (result) {
            assert(false);
            // todo throw
        }
        return value_type == DijSDK_EParamValueTypeDiscreteSet;
    }

    template<typename T>
    std::vector<T> NumericProperty<T>::range() const {
        std::vector<T> allowed;
        if (is_discrete()) {
            unsigned value_count = 0;
            auto result = DijSDK_GetParameterSpec(m_handle, m_id, nullptr, nullptr, nullptr, nullptr, nullptr, &value_count);
            if (result) {
                assert(false);
                // todo throw
            }
            DijSDK_EParamValueType v = DijSDK_EParamValueTypeDiscreteSet;
            allowed = std::vector<T>(value_count, T{});
            result = DijSDK_GetParameterSpec(m_handle, m_id, nullptr, nullptr, nullptr, nullptr, allowed.data(), &value_count);
            if (result) {
                assert(false);
                // todo throw
            }
        }
        else {
            auto p = get_numeric_parameter<T>(m_handle, m_id, dimension(), DijSDK_EParamQueryMin);
            if (p.error) {
                // todo handle
                assert(false);
            }
            allowed.push_back(p.value[0]);
            p = get_numeric_parameter<T>(m_handle, m_id, dimension(), DijSDK_EParamQueryMax);
            if (p.error) {
                // todo handle
                assert(false);
            }
            allowed.push_back(p.value[0]);
        }
        assert(0 < allowed.size());
        return allowed;
    }

    template<typename T>
    bool NumericProperty<T>::allowed(unsigned index) const {
        auto r = range();
        if (is_discrete()) {
            return std::find(r.cbegin(), r.cend(), index) != r.cend();
        }
        else {
            assert(r.size() == 2);
            return r[0] <= index && index <= r[1];
        }
    }

    template<typename T>
    std::vector<T> NumericProperty<T>::get() const {
        auto p = get_numeric_parameter<T>(m_handle, m_id, dimension(), DijSDK_EParamQueryCurrent);
        if (p.error) {
            // todo handle
            assert(false);
        }
        return p.value;
    }

    template<typename T>
    T NumericProperty<T>::get(unsigned index) const {
        auto v = get();
        assert(index < v.size());
        return v[index];
    }

    template<typename T>
    void NumericProperty<T>::set(const std::vector<T> &value) {
        assert(writeable());
        auto p = set_numeric_parameter<T>(m_handle, m_id, value);
        if (p) {
            // todo handle
            assert(false);
        }
    }

    template<typename T>
    void NumericProperty<T>::set(const T &value, unsigned index) {
        assert(writeable());
        auto v = get();
        assert(index < v.size());
        v[index] = value;
        set(v);
    }

    template<typename T>
    std::string NumericProperty<T>::to_string() const {
        std::stringstream ss;
        ss << "dimension: " << dimension() << "\n";
        ss << "writeable: " << writeable() << "\n";
        ss << "is discrete: " << is_discrete() << "\n";
        ss << "range:\n";
        for (const auto &e : range()) {
            ss << "  " << e << "\n";
        }
        return ss.str();
    }

    template<typename T>
    MappedNumericScalarProperty<T>::MappedNumericScalarProperty(NumericProperty<T> prop, std::map<std::string, T> forward) :
        m_property{prop},
        m_forward{forward},
        m_reverse{},
        m_range{}
    {
        for (const auto &e : m_forward) {
            assert(m_property.allowed(e.second));
        }

        std::map<T, std::string> rev;
        for (const auto &e : m_forward) {
            rev.emplace(e.second, e.first);
        }
        m_reverse = rev;

        std::vector<std::string> range;
        for (const auto &e : m_forward) {
            range.emplace_back(e.first);
        }
        m_range = range;
    }

    template<typename T>
    bool MappedNumericScalarProperty<T>::writeable() const {
        return m_property.writeable();
    }

    template<typename T>
    std::vector<std::string> &MappedNumericScalarProperty<T>::range() {
        return m_range;
    }

    template<typename T>
    const std::vector<std::string> &MappedNumericScalarProperty<T>::range() const {
        return m_range;
    }

    template<typename T>
    std::string MappedNumericScalarProperty<T>::get() const {
        auto v = m_property.get(0);
        return m_reverse.at(v);
    }

    template<typename T>
    void MappedNumericScalarProperty<T>::set(const std::string value) {
        auto v = m_forward.at(value);
        m_property.set(v, 0);
    }

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
        assert(num <= (std::numeric_limits<unsigned int>::max)());
        return set_numeric_sdk_parameter<T>(handle, paramId, v.data(), static_cast<unsigned int>(num));
    };
}

#endif