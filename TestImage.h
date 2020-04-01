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

        virtual const unsigned char *get_image_buffer() const;
        virtual unsigned get_number_of_components() const;
        virtual std::string get_component_name(unsigned component) const;
        virtual long get_image_buffer_size() const;
        virtual unsigned get_image_width() const;
        virtual unsigned get_image_height() const;
        virtual unsigned get_image_bytes_per_pixel() const;
        virtual unsigned get_bit_depth() const;
        // get metadata??

    private:
        static const std::vector<unsigned char> M_S_TEST_IMAGE;
        static const std::array<unsigned, 2> M_S_SIZE;
        static const unsigned M_S_BIT_DEPTH;
    };
}

#endif
