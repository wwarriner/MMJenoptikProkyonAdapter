#pragma once

#ifndef PROKYON_IMAGE_INTERFACE_H_
#define PROKYON_IMAGE_INTERFACE_H_

#include <string>

namespace Prokyon {
    class ImageInterface {
    public:
        virtual ~ImageInterface() = default;

        virtual const unsigned char *get_image_buffer() = 0;
        virtual const unsigned char *get_image_buffer() const = 0;
        virtual unsigned get_number_of_components() const = 0;
        virtual std::string get_component_name(unsigned component) const = 0;
        virtual long get_image_buffer_size() const = 0;
        virtual unsigned get_image_width() const = 0;
        virtual unsigned get_image_height() const = 0;
        virtual unsigned get_image_bytes_per_pixel() const = 0;
        virtual unsigned get_bit_depth() const = 0;
        virtual double get_pixel_size_um() const = 0;
        // get metadata??
    };
}

#endif