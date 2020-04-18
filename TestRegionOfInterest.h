#pragma once

#ifndef PROKYON_TEST_REGION_OF_INTEREST_H
#define PROKYON_TEST_REGION_OF_INTEREST_H

#include "CommonDef.h"

#include "RegionOfInterest.h"

namespace Prokyon {
    class TestRegionOfInterest : public RegionOfInterest {
    public:
        TestRegionOfInterest(const ROI &max);

        virtual ROI get() const;
        virtual void set(const ROI roi);
        virtual void clear(); // sets to max size

    private:
        virtual ROI get_max() const;

        ROI m_roi;
        ROI m_max;
    };
}

#endif