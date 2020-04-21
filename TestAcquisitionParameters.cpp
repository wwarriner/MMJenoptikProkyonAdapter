#include "TestAcquisitionParameters.h"

#include "Camera.h"
#include "Parameters.h"

#include <cassert>
#include <cmath>

namespace Prokyon {
    TestAcquisitionParameters::TestAcquisitionParameters() :
        m_bin_size(1),
        m_exposure_time_us(50'000),
        AcquisitionParameters{nullptr} {}

    int TestAcquisitionParameters::get_binning() const {
        return m_bin_size;
    }

    void TestAcquisitionParameters::set_binning(int bin_size) {
        int max = 256;
        if (bin_size < 1) {
            bin_size = 1;
        }
        else if (max < bin_size) {
            bin_size = max;
        }
        m_bin_size = bin_size;
    }

    double TestAcquisitionParameters::get_exposure_ms() const {
        return m_exposure_time_us / 1000.0;
    }

    void TestAcquisitionParameters::set_exposure_ms(double exposure_ms) {
        // check machine limit and convert ms to us
        constexpr auto max_int = (std::numeric_limits<int>::max)();
        auto exposure_us_raw = exposure_ms * 1000.0;
        if (max_int < exposure_us_raw) {
            exposure_us_raw = max_int;
        }
        int exposure_us = std::round(exposure_us_raw);

        // check hardware limits
        int max = 120'000'000; // 120,000,000 us = 2 minutes
        if (exposure_us < 1) {
            exposure_us = 1;
        }
        else if (max < exposure_us) {
            exposure_us = max;
        }
        m_exposure_time_us = exposure_us;
    }
}