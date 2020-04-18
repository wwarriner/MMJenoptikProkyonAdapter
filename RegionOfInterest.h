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

        virtual ROI get() const;
        virtual void set(const ROI roi);
        virtual void clear(); // sets to max size

    private:
        virtual ROI get_max() const;

        ROI m_current_roi;
        Camera *m_p_camera;
    };
}

#endif