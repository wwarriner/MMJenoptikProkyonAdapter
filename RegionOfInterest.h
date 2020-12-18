#pragma once

#ifndef PROKYON_REGION_OF_INTEREST_H
#define PROKYON_REGION_OF_INTEREST_H

#include <array>
#include <exception>
#include <string>
#include <vector>

namespace Prokyon {
    class Camera;

    using ROI = std::array<unsigned int, 4>;

    static const unsigned int X_ind = 0;
    static const unsigned int Y_ind = 1;
    static const unsigned int W_ind = 2;
    static const unsigned int H_ind = 3;

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
        void set(const ROI roi); // throws RegionOfInterestException
        void clear(); // sets to max size, throws RegionOfInterestException

        std::string to_string() const; // throws RegionOfInterestException

    private:
        ROI get_reset_roi() const; // throws RegionOfInterestException
        ROI get_max() const; // throws RegionOfInterestException

        ROI m_roi;
        Camera *m_p_camera;
    };

    class RegionOfInterestException : public std::exception {};
}

#endif