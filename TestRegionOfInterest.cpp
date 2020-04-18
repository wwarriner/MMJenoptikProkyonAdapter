#include "TestRegionOfInterest.h"

namespace Prokyon {
    // public
    TestRegionOfInterest::TestRegionOfInterest(const ROI &max) :
        RegionOfInterest(nullptr),
        m_roi{0, 0, max.at(W_ind), max.at(H_ind)},
        m_max{max} {}

    ROI TestRegionOfInterest::get() const {
        return m_roi;
    }

    void TestRegionOfInterest::set(ROI roi) {
        m_roi = roi;
    }

    void TestRegionOfInterest::clear() {
        auto max = get_max();
        set(ROI{0, 0, max.at(W_ind), max.at(H_ind)});
    }

    // private
    ROI TestRegionOfInterest::get_max() const {
        return m_max;
    }
}