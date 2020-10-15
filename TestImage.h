#pragma once

#ifndef PROKYON_TEST_IMAGE_H_
#define PROKYON_TEST_IMAGE_H_

#include "CommonDef.h"
#include "ImageInterface.h"

#include <array>
#include <string>
#include <vector>

namespace Prokyon {
    class AcquisitionParametersInterface;
    class RegionOfInterestInterface;

    class TestImage : public ImageInterface {
    public:
        TestImage(const AcquisitionParametersInterface *p_ap, const RegionOfInterestInterface *p_roi);
        virtual ~TestImage() = default;

        virtual ImageBuffer get_image_buffer();
        virtual ImageBuffer get_image_buffer() const;
        virtual unsigned get_number_of_components() const;
        virtual std::string get_component_name(unsigned component) const;
        virtual long get_image_buffer_size() const;
        virtual unsigned get_image_width() const;
        virtual unsigned get_image_height() const;
        virtual unsigned get_image_bytes_per_pixel() const;
        virtual unsigned get_bit_depth() const;

        static unsigned int width();
        static unsigned int height();

    private:
        using img = std::vector<unsigned char>;
        using img_seq = std::vector<img>;

        // HACK changes mutable m_img_index, m_current_img
        // required to simulate a volatile hardware buffer
        void update_image() const;
        long get_base_buffer_size() const;
        long to_ind(unsigned int c, unsigned int x, unsigned int y) const;
        static unsigned char to_uchar_color(double value);
        static double to_double_color(unsigned char value);
        static double compute_k(double H_deg, double n);
        static double compute_f(double L, double a, double k);

        const AcquisitionParametersInterface *m_p_acq_param;
        const RegionOfInterestInterface *m_p_roi;
        const unsigned int m_img_count;
        mutable double m_angle_deg;
        mutable unsigned int m_img_index;
        mutable img m_current_img;

        static const img_seq M_S_TEST_IMAGES;
        static const std::array<unsigned, 2> M_S_SIZE;
        static const unsigned M_S_BIT_DEPTH;
        static constexpr unsigned int M_S_COMPONENT_COUNT{4};
    };
}

#endif
