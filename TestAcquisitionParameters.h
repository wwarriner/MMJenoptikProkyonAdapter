#pragma once

#ifndef PROKYON_TEST_ACQUISITION_PARAMETERS_H
#define PROKYON_TEST_ACQUISITION_PARAMETERS_H

#include "AcquisitionParameters.h"
#include "CommonDef.h"

namespace Prokyon {
    class Camera;

    class TestAcquisitionParameters : public AcquisitionParameters {
    public:
        TestAcquisitionParameters();

        virtual int get_binning() const;
        virtual void set_binning(int bin_size);

        virtual double get_exposure_ms() const;
        virtual void set_exposure_ms(double exposure_ms);

    private:
        int m_bin_size;
        int m_exposure_time_us;
    };
}

#endif