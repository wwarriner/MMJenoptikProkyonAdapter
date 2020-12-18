#include "RegionOfInterest.h"

#include "Camera.h"
#include "Parameters.h"

#include <cassert>
#include <limits>
#include <vector>

namespace Prokyon {
    // public
    RegionOfInterest::RegionOfInterest(Camera *p_camera) :
        m_p_camera{p_camera} {
        assert(p_camera != nullptr);
        clear();
    }

    unsigned int RegionOfInterest::x() const {
        return get()[X_ind];
    }

    unsigned int RegionOfInterest::w() const {
        return get()[W_ind];
    }

    unsigned int RegionOfInterest::x_end() const {
        return x() + w();
    }

    unsigned int RegionOfInterest::y() const {
        return get()[Y_ind];
    }

    unsigned int RegionOfInterest::h() const {
        return get()[H_ind];
    }

    unsigned int RegionOfInterest::y_end() const {
        return y() + h();
    }

    ROI RegionOfInterest::get() const {
        return m_roi;
    }

    void RegionOfInterest::set(ROI roi) {
        auto max = get_max();
        for (unsigned int i = 0; i < roi.size(); ++i) {
            assert(roi.at(i) <= max.at(i));
        }
        std::vector<int> value(roi.cbegin(), roi.cend());
        auto result = set_numeric_parameter<int>(*m_p_camera, ParameterIdImageCaptureRoi, value);
        if (result) { throw RegionOfInterestException(); }

        // make sure cached value reflects hardware state
        auto count = std::tuple_size<ROI>::value;
        assert(count < (std::numeric_limits<unsigned int>::max)());
        auto p = get_numeric_parameter<int>(*m_p_camera, ParameterIdImageCaptureRoi, static_cast<unsigned int>(count));
        if (p.error) { throw RegionOfInterestException(); }
        assert(p.value.size() == count);
        auto out = to_unsigned(p.value);
        m_roi = ROI{out[0], out[1], out[2], out[3]};
    }

    void RegionOfInterest::clear() {
        set(get_reset_roi());
    }

    std::string RegionOfInterest::to_string() const {
        std::stringstream ss;
        ss << "Region of Interest (ROI) information:\n";
        ss << "  current: " << x() << ", " << y() << ", " << h() << ", " << w() << "\n";
        auto full = get_reset_roi();
        ss << "  full: " << full[X_ind] << ", " << full[Y_ind] << ", " << full[H_ind] << ", " << full[W_ind] << "\n";
        return ss.str();
    }

    // private
    ROI RegionOfInterest::get_reset_roi() const {
        auto max = get_max();
        return {0, 0, max.at(W_ind), max.at(H_ind)};
    }

    ROI RegionOfInterest::get_max() const {
        auto count = std::tuple_size<ROI>::value;
        assert(count < (std::numeric_limits<unsigned int>::max)());
        auto p = get_numeric_parameter<int>(*m_p_camera, ParameterIdImageCaptureRoi, static_cast<unsigned int>(count), DijSDK_EParamQueryMax);
        if (p.error) { throw RegionOfInterestException(); }
        auto value = to_unsigned(p.value);
        return {value.at(X_ind), value.at(Y_ind), value.at(W_ind), value.at(H_ind)};
    }
}