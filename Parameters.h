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
    class StringProperty {
    public:
        StringProperty(DijSDK_Handle handle, DijSDK_EParamId id);
        std::string get() const;

    private:
        DijSDK_Handle m_handle;
        DijSDK_EParamId m_id;
    };

    class NumericProperty {
    public:
        NumericProperty(DijSDK_Handle handle, DijSDK_EParamId id);

        unsigned dimension() const;
        bool writeable() const;

        bool is_discrete() const;

        std::vector<int> range_discrete() const;
        virtual std::vector<int> range_int() const;
        std::vector<double> range_double() const;

        virtual void normalize_int(int &v) const;
        void normalize_int(std::vector<int> &v) const;

        void normalize_double(double &v) const;
        void normalize_double(std::vector<double> &v) const;

        bool allowed_discrete(int value) const;
        virtual bool allowed_int(int value) const;
        bool allowed_double(double value) const;

        std::vector<int> get_int() const;
        std::vector<double> get_double() const;

        virtual int get_int(unsigned index) const;
        double get_double(unsigned index) const;

        void set(const std::vector<int> &value);
        void set(const std::vector<double> &value);

        virtual void set(const int &value, unsigned index = 0);
        void set(const double &value, unsigned index = 0);

        std::string vector_to_string() const;
        std::string specification_to_string() const;

        enum class Type {
            UnspecifiedType = 0,
            IntType = 1,
            DoubleType = 2
        };
        Type type() const;

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
        T get(unsigned index) const;
        template<typename T>
        void set(const std::vector<T> &value);
        template<typename T>
        void set(const T &value, unsigned index);
        template<typename T>
        std::string value_to_string() const;
        template<typename T>
        std::string range_to_string() const;

        DijSDK_Handle m_handle;
        DijSDK_EParamId m_id;
    };

    class MappedScalarIntProperty {
    public:
        MappedScalarIntProperty(NumericProperty prop, std::map<std::string, int> forward, bool pseudo_mapped = false);

        bool writeable() const;

        std::vector<std::string> &range();
        const std::vector<std::string> &range() const;

        std::string get() const;
        void set(const std::string &value);

    private:
        NumericProperty m_property;
        std::map<std::string, int> m_forward;
        std::map<int, std::string> m_reverse;
        std::vector<std::string> m_range;
        bool m_pseudo_mapped;
    };

    // TODO make like MappedScalarIntProperty
    // use drop-down "on/off"
    class BoolProperty : public NumericProperty {
    public:
        BoolProperty(DijSDK_Handle handle, DijSDK_EParamId id);

        virtual std::vector<int> range_int() const;
        virtual void normalize_int(int &v) const;
        virtual bool allowed_int(int value) const;
        virtual void set(const int &value, unsigned index = 0);
        virtual int get_int(unsigned index) const;
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
        auto p = get_numeric_parameter<T>(m_handle, m_id, dimension(), DijSDK_EParamQueryMin);
        if (p.error) {
            // todo handle
            assert(false);
        }
        range.push_back(p.value[0]);
        p = get_numeric_parameter<T>(m_handle, m_id, dimension(), DijSDK_EParamQueryMax);
        if (p.error) {
            // todo handle
            assert(false);
        }
        range.push_back(p.value[0]);
        assert(range.size() == 2);
        return range;
    }

    template<typename T>
    void NumericProperty::normalize(T &v) const {
        auto r = range<T>();
        if (v < r[0]) {
            v = r[0];
        }
        else if (r[1] < v) {
            v = r[1];
        }
    }

    template<typename T>
    void NumericProperty::normalize(std::vector<T> &v) const {
        auto r = range<T>();
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
        auto p = get_numeric_parameter<T>(m_handle, m_id, dimension(), DijSDK_EParamQueryCurrent);
        if (p.error) {
            // todo handle
            assert(false);
        }
        return p.value;
    }

    template<typename T>
    T NumericProperty::get(unsigned index) const {
        return get<T>()[index];
    }

    template<typename T>
    void NumericProperty::set(const std::vector<T> &value) {
        assert(writeable());
        auto p = set_numeric_parameter<T>(m_handle, m_id, value);
        if (p) {
            // todo handle
            assert(false);
        }
    }

    template<typename T>
    void NumericProperty::set(const T &value, unsigned index) {
        assert(writeable());
        auto v = get<T>();
        assert(index < v.size());
        v[index] = value;
        set(v);
    }

    template<typename T>
    std::string NumericProperty::value_to_string() const {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(4u);
        auto is_first = true;
        for (const auto &v : get<T>()) {
            if (is_first) {
                is_first = false;
                ss << v;
            }
            else {
                ss << ", " << v;
            }
        }
        return ss.str();
    }

    template<typename T>
    std::string NumericProperty::range_to_string() const {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(4u);
        for (const auto &e : range<T>()) {
            ss << "  " << e << "\n";
        }
        return ss.str();
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