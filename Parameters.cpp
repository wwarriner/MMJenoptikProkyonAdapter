#include "Parameters.h"

#include "dijsdk.h"
#include "parameterif.h"

#include <algorithm>

namespace Prokyon {
    PropertyBase::PropertyBase(DijSDK_Handle handle, DijSDK_EParamId id) :
        m_handle{handle},
        m_id{id}
    {}

    bool PropertyBase::exists() const {
        return DijSDK_HasParameter(m_handle, m_id) == E_OK;
    }

    unsigned PropertyBase::dimension() const {
        assert(exists());

        unsigned dimension = 0;
        auto result = DijSDK_GetParameterSpec(m_handle, m_id, nullptr, nullptr, &dimension);
        if (result) { assert(false); }
        return dimension;
    }

    bool PropertyBase::writeable() const {
        assert(exists());

        auto access = static_cast<DijSDK_EParamAccess>(0);
        auto result = DijSDK_GetParameterSpec(m_handle, m_id, nullptr, &access);
        if (result) { assert(false); }
        return (access == DijSDK_EParamAccessReadWrite) || (access == DijSDK_EParamAccessWriteOnly);
    }

    bool PropertyBase::is_discrete() const {
        assert(exists());

        DijSDK_EParamValueType value_type = static_cast<DijSDK_EParamValueType>(0);
        auto result = DijSDK_GetParameterSpec(m_handle, m_id, nullptr, nullptr, nullptr, &value_type);
        if (result) { assert(false); }
        return value_type == DijSDK_EParamValueTypeDiscreteSet;
    }

    std::string PropertyBase::short_specification_to_string() const {
        assert(exists());

        std::stringstream ss;
        ss << dimension_to_string() << " " << DELIMITER << " ";
        ss << type_to_string() << " " << DELIMITER << " ";
        ss << writeable_to_string();
        return ss.str();
    }

    std::string PropertyBase::specification_to_string() const {
        assert(exists());

        std::stringstream ss;
        ss << "type:" << type_to_string() << "\n";
        ss << "dimension: " << dimension_to_string() << "\n";
        ss << "writeable: " << writeable_to_string() << "\n";
        ss << "is discrete: " << bool_to_string(is_discrete()) << "\n";
        ss << "range:\n" << range_to_string() << "\n";
        return ss.str();
    }

    PropertyBase::Type PropertyBase::type() const {
        assert(exists());

        auto mm_type = static_cast<DijSDK_EParamType>(0);
        auto result = DijSDK_GetParameterSpec(m_handle, m_id, &mm_type);
        if (result) { assert(false); }
        auto out = Type::Unspecified;
        switch (mm_type) {
            case DijSDK_EParamTypeNotSpecified:
                out = Type::Unspecified;
                break;
            case DijSDK_EParamTypeBool:
                out = Type::Bool;
                break;
            case DijSDK_EParamTypeInteger:
                if (is_discrete()) {
                    out = Type::Set;
                }
                else {
                    out = Type::Int;
                }
                break;
            case DijSDK_EParamTypeDouble:
                out = Type::Double;
                break;
            case DijSDK_EParamTypeString:
                out = Type::String;
                break;
            default:
                assert(false);
        }
        return out;
    }

    char PropertyBase::delimiter() {
        return DELIMITER;
    }

    std::string PropertyBase::readable_delimiter() {
        std::stringstream ss;
        ss << " " << delimiter() << " ";
        return ss.str();
    }

    DijSDK_Handle &PropertyBase::handle() {
        return m_handle;
    }

    const DijSDK_Handle &PropertyBase::handle() const {
        return m_handle;
    }

    DijSDK_EParamId &PropertyBase::id() {
        return m_id;
    }

    const DijSDK_EParamId &PropertyBase::id() const {
        return m_id;
    }

    std::string PropertyBase::dimension_to_string() const {
        std::string out;
        auto d = dimension();
        if (d == 1) {
            out = "scalar";
        }
        else if (1 < d) {
            std::stringstream ss;
            ss << d << "-vector";
            out = ss.str();
        }
        else {
            assert(false);
        }
        return out;
    }

    std::string PropertyBase::type_to_string() const {
        std::string out;
        switch (type()) {
            case Type::Unspecified:
                out = "unspecified type";
                break;
            case Type::Bool:
                out = "bool";
                break;
            case Type::Int:
                out = "int";
                break;
            case Type::Double:
                out = "double";
                break;
            case Type::String:
                out = "string";
                break;
            case Type::Set:
                out = "discrete set";
                break;
            default:
                assert(false);
        }
        return out;
    }

    std::string PropertyBase::writeable_to_string() const {
        return writeable() ? "rw" : "ro";
    }

    std::string PropertyBase::bool_to_string(bool v) {
        return v ? "true" : "false";
    }

    StringProperty::StringProperty(DijSDK_Handle handle, DijSDK_EParamId id) :
        PropertyBase(handle, id) {
        assert(type() == Type::String);
    }

    std::string StringProperty::get() const {
        assert(exists());
        assert(type() == Type::String);

        auto p = get_string_parameter(handle(), id(), 1024u);
        if (p.error) { assert(false); }
        return p.value;
    }

    std::string StringProperty::range_to_string() const {
        assert(exists());
        assert(type() == Type::String);
        return "N/A";
    }

    NumericProperty::NumericProperty(DijSDK_Handle handle, DijSDK_EParamId id) :
        PropertyBase(handle, id) {
        assert(
            type() == Type::Double
            || type() == Type::Int
            || type() == Type::Set
            || type() == Type::Bool
        );
    }

    std::vector<int> NumericProperty::range_int() const {
        assert(exists());
        std::vector<int> out;
        if (type() == Type::Int) {
            out = range<int>();
        }
        else if (type() == Type::Bool) {
            out = {0, 1};
        }
        else {
            assert(false);
        }
        return out;
    }

    std::vector<double> NumericProperty::range_double() const {
        assert(exists());
        assert(type() == Type::Double);
        return range<double>();
    }

    void NumericProperty::clip_int(std::vector<int> &v) const {
        assert(exists());
        assert(type() == Type::Int);
        clip<int>(v);
    }

    void NumericProperty::clip_double(std::vector<double> &v) const {
        assert(exists());
        assert(type() == Type::Double);
        clip<double>(v);
    }

    bool NumericProperty::allowed_int(int value) const {
        assert(exists());
        auto out = false;
        if (type() == Type::Int) {
            out = allowed<int>(value);
        }
        else if (type() == Type::Bool) {
            out = (value == 0) || (value == 1);
        }
        else {
            assert(false);
        }
        return out;
    }

    bool NumericProperty::allowed_double(double value) const {
        assert(exists());
        assert(type() == Type::Double);
        return allowed<double>(value);
    }

    std::vector<int> NumericProperty::get_int() const {
        assert(exists());
        assert(type() == Type::Int || type() == Type::Bool || type() == Type::Set);
        return get<int>();
    }

    std::vector<double> NumericProperty::get_double() const {
        assert(exists());
        assert(type() == Type::Double);
        return get<double>();
    }

    std::string NumericProperty::get_as_string() const {
        assert(exists());
        std::string out;
        switch (type()) {
            case Type::Int:
                out = vec_to_string(get<int>());
                break;
            case Type::Double:
                out = vec_to_string(get<double>());
                break;
            default:
                assert(false);
        }
        return out;
    }

    void NumericProperty::set(const std::vector<int> &value) {
        assert(exists());
        assert(writeable());
        assert(type() == Type::Int || type() == Type::Bool || type() == Type::Set);
        set<int>(value);
    }

    void NumericProperty::set(const std::vector<double> &value) {
        assert(exists());
        assert(writeable());
        assert(type() == Type::Double);
        set<double>(value);
    }

    std::string NumericProperty::range_to_string() const {
        std::string out;
        switch (type()) {
            case Type::Int:
                out = value_range_to_string(range<int>());
                break;
            case Type::Double:
                out = value_range_to_string(range<double>());
                break;
            default:
                assert(false);
        }
        return out;
    }

    DiscreteSetProperty::DiscreteSetProperty(DijSDK_Handle handle, DijSDK_EParamId id, std::map<std::string, int> forward, bool pseudo_mapped) :
        NumericProperty(handle, id),
        m_forward{forward},
        m_reverse{},
        m_range{},
        m_pseudo_mapped{pseudo_mapped}
    {
        for (const auto &e : m_forward) {
            if (m_pseudo_mapped) {
                assert(NumericProperty::allowed_int(e.second));
            }
            else {
                assert(allowed_discrete(e.second));
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

        assert(
            type() == Type::Set
            || (type() == Type::Int && m_pseudo_mapped)
            || (type() == Type::Bool && m_pseudo_mapped)
        );
    }

    std::vector<int> DiscreteSetProperty::range_discrete() const {
        assert(exists());
        assert(type() == Type::Set);

        unsigned value_count = 0;
        auto result = DijSDK_GetParameterSpec(handle(), id(), nullptr, nullptr, nullptr, nullptr, nullptr, &value_count);
        if (result) { assert(false); }
        assert(0 < value_count);

        std::vector<int> range(value_count, 0);
        result = DijSDK_GetParameterSpec(handle(), id(), nullptr, nullptr, nullptr, nullptr, range.data(), &value_count);
        if (result) { assert(false); }
        assert(0 < range.size());

        return range;
    }

    bool DiscreteSetProperty::allowed_discrete(int value) const {
        assert(exists());
        assert(type() == Type::Set);
        auto r = range_discrete();
        return std::find(r.cbegin(), r.cend(), value) != r.cend();
    }

    std::vector<std::string> &DiscreteSetProperty::range() {
        return m_range;
    }

    const std::vector<std::string> &DiscreteSetProperty::range() const {
        return m_range;
    }

    std::string DiscreteSetProperty::get() const {
        auto v = NumericProperty::get_int()[0];
        return m_reverse.at(v);
    }

    void DiscreteSetProperty::set(const std::string &value) {
        assert(exists());
        auto v = m_forward.at(value);
        auto old = NumericProperty::get_int();
        old[0] = v;
        NumericProperty::set(old);
    }

    std::string DiscreteSetProperty::range_to_string() const {
        assert(exists());
        std::stringstream ss;
        auto is_first = true;
        for (const auto &v : m_range) {
            if (is_first) {
                is_first = false;
                ss << v;
            }
            else {
                ss << "\n" << v;
            }
        }
        return ss.str();
    }

    BoolProperty::BoolProperty(DijSDK_Handle handle, DijSDK_EParamId id) :
        DiscreteSetProperty(handle, id, {{"false", 0}, {"true", 1}}, true) {
        assert(type() == Type::Bool);
    }

    // free functions
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