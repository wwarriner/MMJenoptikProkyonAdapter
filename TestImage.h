#pragma once

#ifndef PROKYON_TEST_IMAGE_H_
#define PROKYON_TEST_IMAGE_H_

#include "ImageInterface.h"

#include <array>
#include <string>
#include <vector>

namespace Prokyon {
    class TestImage : public ImageInterface {
    public:
        virtual ~TestImage() = default;

        virtual const unsigned char *get_image_buffer();
        virtual const unsigned char *get_image_buffer() const;
        virtual unsigned get_number_of_components() const;
        virtual std::string get_component_name(unsigned component) const;
        virtual long get_image_buffer_size() const;
        virtual unsigned get_image_width() const;
        virtual unsigned get_image_height() const;
        virtual unsigned get_image_bytes_per_pixel() const;
        virtual unsigned get_bit_depth() const;
        virtual double get_pixel_size_um() const;

    private:
        using img = std::vector<unsigned char>;
        using img_seq = std::vector<img>;

        img current() const;
        void cycle();

        unsigned int m_img_index;
        static const img_seq M_S_TEST_IMAGES;
        static const std::array<unsigned, 2> M_S_SIZE;
        static const unsigned M_S_BIT_DEPTH;
    };
}

#endif
