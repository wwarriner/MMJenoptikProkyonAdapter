#pragma once

#ifndef PROKYON_IMAGE_H_
#define PROKYON_IMAGE_H_

#include "CommonDef.h"
#include "ImageInterface.h"

#include <map>
#include <string>
#include <vector>

namespace Prokyon {
    class Image : public ImageInterface {
    public:
        Image(CameraHandle camera_handle);
        virtual ~Image();

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
        using NameMap = std::map<unsigned, std::string>;

    private:
        static unsigned get_component_count(ImageHandle image);
        static NameMap get_component_name_map(unsigned count);
        static unsigned get_bits_per_channel(ImageHandle image);
        static std::vector<unsigned> get_size(ImageHandle image);
        static double get_pixel_size_um(ImageHandle image);

    private:
        ImageHandle m_handle;
        ImageBuffer m_p_data;
        unsigned m_component_count;
        NameMap m_component_names;
        unsigned m_bits_per_channel;
        std::vector<unsigned> m_size;
        double m_pixel_size_um;

        static const NameMap M_S_RGBA_COMPONENT_NAMES;
        static const NameMap M_S_GRAY_COMPONENT_NAMES;
    };
}

#endif
