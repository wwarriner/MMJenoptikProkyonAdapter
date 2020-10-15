#pragma once

#ifndef PROKYON_IMAGE_H_
#define PROKYON_IMAGE_H_

#include "CommonDef.h"
#include "ImageInterface.h"

#include <map>
#include <string>
#include <vector>

namespace Prokyon {
    class Camera;

    class Image : public ImageInterface {
    public:
        Image(Camera *p_camera);

        virtual ImageBuffer get_image_buffer();
        virtual ImageBuffer get_image_buffer() const;
        virtual unsigned get_number_of_components() const;
        virtual std::string get_component_name(unsigned component) const;
        virtual long get_image_buffer_size() const;
        virtual unsigned get_image_width() const;
        virtual unsigned get_image_height() const;
        virtual unsigned get_image_bytes_per_pixel() const;
        virtual unsigned get_bit_depth() const;

    private:
        using NameMap = std::map<unsigned, std::string>;
        using ImageData = std::vector<unsigned char>;
        using Size = std::vector<unsigned>;

    private:
        static unsigned extract_component_count(ImageHandle image);
        static NameMap build_component_name_map(unsigned component_count);
        static unsigned extract_bits_per_channel(ImageHandle image);
        static unsigned compute_bits_per_pixel(unsigned component_count, unsigned bits_per_channel);
        static unsigned compute_bytes_per_pixel(unsigned bits_per_pixel);
        static Size extract_size(ImageHandle image);
        static long compute_image_bytes(std::vector<unsigned> size, unsigned component_count, unsigned bytes_per_pixel);
        static ImageData copy_image_data(void *p_data, long bytes);

    private:
        unsigned m_component_count;
        NameMap m_component_names;
        unsigned m_bits_per_channel;
        unsigned m_bits_per_pixel;
        unsigned m_bytes_per_pixel;
        Size m_size;
        long m_bytes;
        ImageData m_data;

        static const NameMap M_S_RGBA_COMPONENT_NAMES;
        static const NameMap M_S_GRAY_COMPONENT_NAMES;
    };
}

#endif
