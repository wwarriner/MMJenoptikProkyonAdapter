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
#include <iomanip>
#include <sstream>
#include <iostream>

using DijSDK_Handle = void *;
using error_t = int;

namespace Prokyon {
    class PropertyBase {
    public:
        PropertyBase(DijSDK_Handle handle, DijSDK_EParamId id);

        bool exists() const;

        unsigned dimension() const;
        bool writeable() const;
        bool is_discrete() const;

        std::string short_specification_to_string() const;
        std::string specification_to_string() const;

        enum class Type {
            Unspecified = 0,
            Bool = 1,
            Int = 2,
            Double = 3,
            String = 4,
            Set = 5,
        };
        Type type() const;

        char delimiter() const;
        std::string readable_delimiter() const;

    protected:
        DijSDK_Handle &handle();
        const DijSDK_Handle &handle() const;
        DijSDK_EParamId &id();
        const DijSDK_EParamId &id() const;

        virtual std::string range_to_string() const = 0;
        template<typename T>
        std::string vec_to_string(const std::vector<T> values) const;

        template<typename T>
        static std::string value_range_to_string(std::vector<T> range);

        static const char DELIMITER = '|';
        static const unsigned PRECISION = 4u;

    private:
        std::string dimension_to_string() const;
        std::string type_to_string() const;
        std::string writeable_to_string() const;

        static std::string bool_to_string(bool v);
        template<typename T>
        static std::string vec_to_string(const std::vector<T> values, const char delimiter, const unsigned precision);

    private:
        DijSDK_Handle m_handle;
        DijSDK_EParamId m_id;
    };

    template<typename T>
    std::string PropertyBase::vec_to_string(const std::vector<T> values) const {
        return vec_to_string(values, DELIMITER, PRECISION);
    }

    template<typename T>
    std::string PropertyBase::value_range_to_string(const std::vector<T> range) {
        std::stringstream ss;
        assert(range.size() == 2);
        ss << "[" << range[0] << ", " << range[1] << "]";
        return ss.str();
    }

    template<typename T>
    std::string PropertyBase::vec_to_string(const std::vector<T> values, const char delimiter, const unsigned precision) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(precision);
        auto is_first = true;
        for (const auto &v : values) {
            if (is_first) {
                is_first = false;
                ss << v;
            }
            else {
                ss << " " << delimiter << " " << v;
            }
        }
        return ss.str();
    }

    class StringProperty : public PropertyBase {
    public:
        StringProperty(DijSDK_Handle handle, DijSDK_EParamId id);
        std::string get() const;
        // no set because no useful read-only properties at this time

    protected:
        virtual std::string range_to_string() const;
    };

    class NumericProperty : public PropertyBase {
    public:
        NumericProperty(DijSDK_Handle handle, DijSDK_EParamId id);

        std::vector<int> range_int() const;
        std::vector<double> range_double() const;

        void normalize_int(std::vector<int> &v) const;
        void normalize_double(std::vector<double> &v) const;

        bool allowed_int(int value) const;
        bool allowed_double(double value) const;

        std::vector<int> get_int() const;
        std::vector<double> get_double() const;

        void set(const std::vector<int> &value);
        void set(const std::vector<double> &value);

        std::string vector_to_string() const;

    protected:
        virtual std::string range_to_string() const;

    protected:
        template<typename T>
        std::vector<T> range() const;
        template<typename T>
        void normalize(T &v) const;
        template<typename T>
        void normalize(std::vector<T> &v) const;
        template<typename T>
        bool allowed(const T value) const;
        template<typename T>
        std::vector<T> get() const;
        template<typename T>
        void set(const std::vector<T> &value);
    };

    class DiscreteSetProperty : public NumericProperty {
    public:
        DiscreteSetProperty(DijSDK_Handle handle, DijSDK_EParamId id, std::map<std::string, int> forward, bool pseudo_mapped = false);

        std::vector<std::string> &range();
        const std::vector<std::string> &range() const;

        std::string get() const;
        void set(const std::string &value);

    protected:
        virtual std::string range_to_string() const;

    private:
        std::vector<int> range_discrete() const;
        bool allowed_discrete(int value) const;

    private:
        std::map<std::string, int> m_forward;
        std::map<int, std::string> m_reverse;
        std::vector<std::string> m_range;
        bool m_pseudo_mapped;
    };

    class BoolProperty : public DiscreteSetProperty {
    public:
        BoolProperty(DijSDK_Handle handle, DijSDK_EParamId id);
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
    std::vector<T> NumericProperty::range() const {
        std::vector<T> range;

        auto p = get_numeric_parameter<T>(handle(), id(), dimension(), DijSDK_EParamQueryMin);
        if (p.error) { assert(false); }
        range.push_back(p.value[0]);

        p = get_numeric_parameter<T>(handle(), id(), dimension(), DijSDK_EParamQueryMax);
        if (p.error) { assert(false); }
        range.push_back(p.value[0]);

        assert(range[0] <= range[1]);
        assert(range.size() == 2);
        return range;
    }

    template<typename T>
    void NumericProperty::normalize(T &v) const {
        auto r = range<T>();
        if (v < r[0]) { v = r[0]; }
        if (r[1] < v) { v = r[1]; }
    }

    template<typename T>
    void NumericProperty::normalize(std::vector<T> &v) const {
        for (auto i = 0; i < v.size(); ++i) {
            normalize<T>(v[i]);
        }
    }

    template<typename T>
    bool NumericProperty::allowed(const T value) const {
        auto r = range<T>();
        assert(r.size() == 2);
        assert(r[0] <= r[1]);
        return r[0] <= value && value <= r[1];
    }

    template<typename T>
    std::vector<T> NumericProperty::get() const {
        auto p = get_numeric_parameter<T>(handle(), id(), dimension(), DijSDK_EParamQueryCurrent);
        if (p.error) { assert(false); }
        return p.value;
    }

    template<typename T>
    void NumericProperty::set(const std::vector<T> &value) {
        assert(writeable());
        auto p = set_numeric_parameter<T>(handle(), id(), value);
        if (p) {
            // todo handle
            assert(false);
        }
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