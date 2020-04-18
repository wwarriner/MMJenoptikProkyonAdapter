#pragma once

#ifndef PROKYON_ACQUISITION_PARAMETERS_H
#define PROKYON_ACQUISITION_PARAMETERS_H

#include "CommonDef.h"

namespace Prokyon {
    class Camera;
    class RegionOfInterest;

    class AcquisitionParameters {
    public:
        AcquisitionParameters(Camera *p_camera, RegionOfInterest *p_roi);

        int get_binning() const;
        void set_binning(int bin_size);

        double get_exposure() const;
        void set_exposure(double exposure_ms);

        ROI get_roi() const;
        void set_roi(const ROI roi);
        void clear_roi();

    private:
        Camera *m_p_camera;
        RegionOfInterest *m_p_roi;
    };
}

#endif