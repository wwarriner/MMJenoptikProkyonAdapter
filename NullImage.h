#pragma once

#ifndef PROKYON_NULL_IMAGE_H_
#define PROKYON_NULL_IMAGE_H_

#include "CommonDef.h"
#include "ImageInterface.h"

namespace Prokyon {
    class NullImage : public ImageInterface {
    public:
        virtual ImageBuffer get_image_buffer();
        virtual ImageBuffer get_image_buffer() const;
        virtual unsigned get_number_of_components() const;
        virtual std::string get_component_name(unsigned component) const;
        virtual long get_image_buffer_size() const;
        virtual unsigned get_image_width() const;
        virtual unsigned get_image_height() const;
        virtual unsigned get_image_bytes_per_pixel() const;
        virtual unsigned get_bit_depth() const;
    };
}

#endif