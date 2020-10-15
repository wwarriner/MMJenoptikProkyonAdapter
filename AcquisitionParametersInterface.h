#pragma once

#ifndef PROKYON_ACQUISITION_PARAMETERS_INTERFACE_H
#define PROKYON_ACQUISITION_PARAMETERS_INTERFACE_H

#include "CommonDef.h"

namespace Prokyon {
    class Camera;

    class AcquisitionParametersInterface {
    public:
        virtual ~AcquisitionParametersInterface() = default;

        virtual int get_binning() const = 0;
        virtual void set_binning(int bin_size) = 0;

        virtual double get_exposure_ms() const = 0;
        virtual void set_exposure_ms(double exposure_ms) = 0;
    };
}

#endif