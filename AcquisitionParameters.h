#pragma once

#ifndef PROKYON_ACQUISITION_PARAMETERS_H
#define PROKYON_ACQUISITION_PARAMETERS_H

#include "CommonDef.h"

namespace Prokyon {
    class Camera;

    class AcquisitionParameters {
    public:
        AcquisitionParameters(Camera *p_camera);

        virtual int get_binning() const;
        virtual void set_binning(int bin_size);

        virtual double get_exposure_ms() const;
        virtual void set_exposure_ms(double exposure_ms);

    private:
        Camera *m_p_camera;
    };
}

#endif