#include "Parameters.h"

#include "dijsdk.h"
#include "parameterif.h"

#include <algorithm>

namespace Prokyon {
    StringProperty::StringProperty(DijSDK_Handle handle, DijSDK_EParamId id) :
        m_handle{handle},
        m_id{id}
    {}

    std::string StringProperty::get() const {
        auto p = get_string_parameter(m_handle, m_id, 1024u);
        if (p.error) {
            // todo handle
            assert(false);
        }
        return p.value;
    }

    NumericProperty::NumericProperty(DijSDK_Handle handle, DijSDK_EParamId id) :
        m_handle{handle},
        m_id{id}
    {}

    unsigned NumericProperty::dimension() const {
        unsigned dimension = 0;
        auto result = DijSDK_GetParameterSpec(m_handle, m_id, nullptr, nullptr, &dimension);
        if (result) {
            assert(false);
            // todo throw
        }
        return dimension;
    }

    bool NumericProperty::writeable() const {
        DijSDK_EParamAccess access = static_cast<DijSDK_EParamAccess>(0);
        auto result = DijSDK_GetParameterSpec(m_handle, m_id, nullptr, &access);
        if (result) {
            assert(false);
            // todo throw
        }
        return (access == DijSDK_EParamAccessReadWrite) || (access == DijSDK_EParamAccessWriteOnly);
    }

    bool NumericProperty::is_discrete() const {
        DijSDK_EParamValueType value_type = static_cast<DijSDK_EParamValueType>(0);
        auto result = DijSDK_GetParameterSpec(m_handle, m_id, nullptr, nullptr, nullptr, &value_type);
        if (result) {
            assert(false);
            // todo throw
        }
        return value_type == DijSDK_EParamValueTypeDiscreteSet;
    }

    std::vector<int> NumericProperty::range_discrete() const {
        assert(type() == Type::IntType);
        unsigned value_count = 0;
        auto result = DijSDK_GetParameterSpec(m_handle, m_id, nullptr, nullptr, nullptr, nullptr, nullptr, &value_count);
        if (result) {
            assert(false);
            // todo throw
        }
        assert(0 < value_count);
        std::vector<int> range(value_count, 0);
        result = DijSDK_GetParameterSpec(m_handle, m_id, nullptr, nullptr, nullptr, nullptr, range.data(), &value_count);
        if (result) {
            assert(false);
            // todo throw
        }
        assert(0 < range.size());
        return range;
    }

    std::vector<int> NumericProperty::range_int() const {
        assert(type() == Type::IntType);
        return range<int>();
    }

    std::vector<double> NumericProperty::range_double() const {
        assert(type() == Type::DoubleType);
        return range<double>();
    }

    void NumericProperty::normalize_int(int &v) const {
        assert(type() == Type::IntType);
        normalize<int>(v);
    }

    void NumericProperty::normalize_double(double &v) const {
        assert(type() == Type::DoubleType);
        normalize<double>(v);
    }

    void NumericProperty::normalize_int(std::vector<int> &v) const {
        assert(type() == Type::IntType);
        normalize<int>(v);
    }

    void NumericProperty::normalize_double(std::vector<double> &v) const {
        assert(type() == Type::DoubleType);
        normalize<double>(v);
    }

    bool NumericProperty::allowed_discrete(int value) const {
        assert(type() == Type::IntType);
        auto r = range_discrete();
        return std::find(r.cbegin(), r.cend(), value) != r.cend();
    }

    bool NumericProperty::allowed_int(int value) const {
        assert(type() == Type::IntType);
        return allowed<int>(value);
    }

    bool NumericProperty::allowed_double(double value) const {
        assert(type() == Type::DoubleType);
        return allowed<double>(value);
    }

    std::vector<int> NumericProperty::get_int() const {
        assert(type() == Type::IntType);
        return get<int>();
    }

    std::vector<double> NumericProperty::get_double() const {
        assert(type() == Type::DoubleType);
        return get<double>();
    }

    int NumericProperty::get_int(unsigned index) const {
        assert(type() == Type::IntType);
        return get<int>(index);
    }

    double NumericProperty::get_double(unsigned index) const {
        assert(type() == Type::DoubleType);
        return get<double>(index);
    }

    void NumericProperty::set(const std::vector<int> &value) {
        assert(type() == Type::IntType);
        set<int>(value);
    }

    void NumericProperty::set(const std::vector<double> &value) {
        assert(type() == Type::DoubleType);
        set<double>(value);
    }

    void NumericProperty::set(const int &value, unsigned index) {
        assert(type() == Type::IntType);
        set<int>(value, index);
    }

    void NumericProperty::set(const double &value, unsigned index) {
        assert(type() == Type::DoubleType);
        set<double>(value, index);
    }

    std::string NumericProperty::vector_to_string() const {
        std::stringstream ss;
        switch (type()) {
            case Type::IntType:
                ss << value_to_string<int>();
                break;
            case Type::DoubleType:
                ss << value_to_string<double>();
                break;
            default:
                assert(false);
        }
        return ss.str();
    }

    std::string NumericProperty::specification_to_string() const {
        std::stringstream ss;
        ss << "dimension: " << dimension() << "\n";
        ss << "writeable: " << writeable() << "\n";
        ss << "is discrete: " << is_discrete() << "\n";
        ss << "range:\n";
        switch (type()) {
            case Type::IntType:
                ss << range_to_string<int>();
                break;
            case Type::DoubleType:
                ss << range_to_string<double>();
                break;
            default:
                assert(false);
        }
        return ss.str();
    }

    NumericProperty::Type NumericProperty::type() const {
        DijSDK_EParamType mm_type = static_cast<DijSDK_EParamType>(0);
        auto result = DijSDK_GetParameterSpec(m_handle, m_id, &mm_type);
        if (result) {
            assert(false);
            // todo throw
        }
        Type out = Type::UnspecifiedType;
        switch (mm_type) {
            case DijSDK_EParamTypeBool:
            case DijSDK_EParamTypeInteger:
                out = Type::IntType;
                break;
            case DijSDK_EParamTypeDouble:
                out = Type::DoubleType;
                break;
            default:
                assert(false);
        }
        return out;
    }

    BoolProperty::BoolProperty(DijSDK_Handle handle, DijSDK_EParamId id) :
        NumericProperty(handle, id) {}

    std::vector<int> BoolProperty::range_int() const {
        assert(type() == NumericProperty::Type::IntType);
        return {0, 1};
    }

    void BoolProperty::normalize_int(int &v) const {
        auto r = range_int();
        if (v < r[0]) {
            v = r[0];
        }
        else if (r[1] < v) {
            v = r[1];
        }
    }

    bool BoolProperty::allowed_int(int value) const {
        return value == 0 || value == 1;
    }

    void BoolProperty::set(const int &value, unsigned index) {
        assert(writeable());
        assert(index == 0);
        NumericProperty::set<int>(std::vector<int>{value});
    }

    int BoolProperty::get_int(unsigned index) const {
        return NumericProperty::get<int>(index);
    }

    MappedScalarIntProperty::MappedScalarIntProperty(NumericProperty prop, std::map<std::string, int> forward, bool pseudo_mapped) :
        m_property{prop},
        m_forward{forward},
        m_reverse{},
        m_range{},
        m_pseudo_mapped{pseudo_mapped}
    {
        for (const auto &e : m_forward) {
            if (m_pseudo_mapped) {
                assert(m_property.allowed_int(e.second));
            }
            else {
                assert(m_property.allowed_discrete(e.second));
            }
        }

        std::map<int, std::string> rev;
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

    bool MappedScalarIntProperty::writeable() const {
        return m_property.writeable();
    }

    std::vector<std::string> &MappedScalarIntProperty::range() {
        return m_range;
    }

    const std::vector<std::string> &MappedScalarIntProperty::range() const {
        return m_range;
    }

    std::string MappedScalarIntProperty::get() const {
        auto v = m_property.get_int(0);
        return m_reverse.at(v);
    }

    void MappedScalarIntProperty::set(const std::string &value) {
        auto v = m_forward.at(value);
        m_property.set(v, 0);
    }

    StringParameter get_string_parameter(DijSDK_Handle handle, DijSDK_EParamId param_id, unsigned int length) {
        std::vector<char> value(length, char{});
        auto result = DijSDK_GetStringParameter(handle, param_id, value.data(), length);
        auto it = std::find(value.cbegin(), value.cend(), '\0');
        return {std::string(value.cbegin(), it), result};
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
        std::vector<typename std::make_unsigned<int>::type> unsigned_value(signed_value.size());
        auto transform_function = [](const int v) -> typename std::make_unsigned<int>::type {return to_unsigned(v); };
        std::transform(signed_value.cbegin(), signed_value.cend(), unsigned_value.begin(), transform_function);
        return unsigned_value;
    }
}