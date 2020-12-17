#include "AcquisitionParameters.h"

#include "Camera.h"
#include "Parameters.h"

#include <cassert>
#include <cmath>

// TODO refactor this class into NumericProperty<T>

namespace Prokyon {
    AcquisitionParameters::AcquisitionParameters(Camera *p_camera) : m_p_camera{p_camera} {}

    int AcquisitionParameters::get_binning() const {
        auto p = get_numeric_parameter<int>(*m_p_camera, ParameterIdImageModeAveraging, 1);
        if (p.error) {
            // TODO handle error
            assert(false);
        }
        return p.value.at(0);
    }

    double AcquisitionParameters::get_exposure_ms() const {
        auto p = get_numeric_parameter<int>(*m_p_camera, ParameterIdImageCaptureExposureTimeUsec, 1);
        if (p.error) {
            // TODO handle error
            assert(false);
        }
        auto exposure_us = p.value.at(0);
        return exposure_us / 1000.0;
    }

    void AcquisitionParameters::set_exposure_ms(double exposure_ms) {
        // check machine limit and convert ms to us
        constexpr auto max_int = (std::numeric_limits<int>::max)();
        auto exposure_us_raw = exposure_ms * 1000.0;
        if (max_int < exposure_us_raw) {
            exposure_us_raw = max_int;
        }
        int exposure_us = static_cast<int>(std::round(exposure_us_raw));

        // check hardware limits
        auto p = get_numeric_parameter<int>(*m_p_camera, ParameterIdImageCaptureExposureTimeUsec, 1, DijSDK_EParamQueryMax);
        if (p.error) {
            // TODO handle error
            assert(false);
        }
        auto max = p.value.at(0);
        if (exposure_us < 1) {
            exposure_us = 1;
        }
        else if (max < exposure_us) {
            exposure_us = max;
        }
        auto result = set_numeric_parameter<int>(*m_p_camera, ParameterIdImageCaptureExposureTimeUsec, std::vector<int>{exposure_us});
        if (result) {
            // TODO handle error
            assert(false);
        }
    }

    std::string AcquisitionParameters::to_string() const {
        std::stringstream ss;
        ss << "Acquisition Parameters:\n";
        ss << "  address: " << this << "\n";
        ss << "  binning: " << get_binning() << "\n";
        ss << "  exposure time (ms): " << get_exposure_ms() << "\n";
        return ss.str();
    }
}