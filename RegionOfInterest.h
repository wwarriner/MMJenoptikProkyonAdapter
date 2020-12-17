#pragma once

#ifndef PROKYON_REGION_OF_INTEREST_H
#define PROKYON_REGION_OF_INTEREST_H

#include "CommonDef.h"

#include <vector>

namespace Prokyon {
    class Camera;

    class RegionOfInterest {
    public:
        RegionOfInterest(Camera *p_camera);

        unsigned int x() const;
        unsigned int w() const;
        unsigned int x_end() const;
        unsigned int y() const;
        unsigned int h() const;
        unsigned int y_end() const;

        ROI get() const;
        void set(const ROI roi);
        void clear(); // sets to max size

        std::string to_string() const;

    private:
        ROI get_reset_roi() const;
        ROI get_max() const;

        ROI m_roi;
        Camera *m_p_camera;
    };
}

#endif