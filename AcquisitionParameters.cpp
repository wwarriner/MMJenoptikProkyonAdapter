#include "AcquisitionParameters.h"

#include "RegionOfInterest.h"
#include "Camera.h"

#include <cassert>

namespace Prokyon {
    AcquisitionParameters::AcquisitionParameters(Camera *p_camera, RegionOfInterest *p_roi) :
        m_p_roi{p_roi} {}

    int AcquisitionParameters::get_binning() const {
        assert(false);
    }

    void AcquisitionParameters::set_binning(int bin_size) {
        assert(false);
    }

    double AcquisitionParameters::get_exposure() const {
        assert(false);
    }

    void AcquisitionParameters::set_exposure(double exposure_ms) {
        assert(false);
    }

    ROI AcquisitionParameters::get_roi() const {
        return m_p_roi->get();
    }

    void AcquisitionParameters::set_roi(const ROI roi) {
        m_p_roi->set(roi);
    }

    void AcquisitionParameters::clear_roi() {
        m_p_roi->clear();
    }
}