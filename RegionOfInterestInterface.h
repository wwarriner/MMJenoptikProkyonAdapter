#pragma once

#ifndef PROKYON_TEST_REGION_OF_INTEREST_INTERFACE_H
#define PROKYON_TEST_REGION_OF_INTEREST_INTERFACE_H

#include "CommonDef.h"

namespace Prokyon {
    class RegionOfInterestInterface {
    public:
        virtual ~RegionOfInterestInterface() = default;

        virtual unsigned int x() const = 0;
        virtual unsigned int w() const = 0;
        virtual unsigned int x_end() const = 0;
        virtual unsigned int y() const = 0;
        virtual unsigned int h() const = 0;
        virtual unsigned int y_end() const = 0;

        virtual ROI get() const = 0;
        virtual void set(const ROI roi) = 0;
        virtual void clear() = 0; // sets to max size
    };
}

#endif