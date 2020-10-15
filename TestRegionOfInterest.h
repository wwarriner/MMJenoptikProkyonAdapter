#pragma once

#ifndef PROKYON_TEST_REGION_OF_INTEREST_H
#define PROKYON_TEST_REGION_OF_INTEREST_H

#include "CommonDef.h"
#include "RegionOfInterestInterface.h"

namespace Prokyon {
    class TestRegionOfInterest : public RegionOfInterestInterface {
    public:
        TestRegionOfInterest(const ROI &max);

        virtual unsigned int x() const;
        virtual unsigned int w() const;
        virtual unsigned int x_end() const;
        virtual unsigned int y() const;
        virtual unsigned int h() const;
        virtual unsigned int y_end() const;

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