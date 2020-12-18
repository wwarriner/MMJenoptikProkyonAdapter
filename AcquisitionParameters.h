#pragma once

#ifndef PROKYON_ACQUISITION_PARAMETERS_H
#define PROKYON_ACQUISITION_PARAMETERS_H

#include <exception>
#include <string>

namespace Prokyon {
    class Camera;

    class AcquisitionParameters {
    public:
        AcquisitionParameters(Camera *p_camera);

        int get_binning() const; // throws AcquisitionParametersException
        // no set, hardware does not allow this directly
        // please see Image Mode property in micromanager

        double get_exposure_ms() const; // throws AcquisitionParametersException
        void set_exposure_ms(double exposure_ms); // throws AcquisitionParametersException

        std::string to_string() const;

    private:
        Camera *m_p_camera;
    };

    class AcquisitionParametersException : public std::exception {};
}

#endif