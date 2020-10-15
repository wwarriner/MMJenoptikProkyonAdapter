#include "TestRegionOfInterest.h"

namespace Prokyon {
    // public
    TestRegionOfInterest::TestRegionOfInterest(const ROI &max) :
        m_roi{0, 0, max.at(W_ind), max.at(H_ind)},
        m_max{max} {}

    unsigned int TestRegionOfInterest::x() const {
        return get()[X_ind];
    }

    unsigned int TestRegionOfInterest::w() const {
        return get()[W_ind];
    }

    unsigned int TestRegionOfInterest::x_end() const {
        return x() + w();
    }

    unsigned int TestRegionOfInterest::y() const {
        return get()[Y_ind];
    }

    unsigned int TestRegionOfInterest::h() const {
        return get()[H_ind];
    }

    unsigned int TestRegionOfInterest::y_end() const {
        return y() + h();
    }

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