#pragma once

#ifndef PROKYON_IMAGE_H_
#define PROKYON_IMAGE_H_

#include "CommonDef.h"

#include <map>
#include <string>
#include <vector>

namespace Prokyon {
    class Camera;

    class Image {
    public:
        Image(Camera *p_camera);

        void acquire();

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
        using ImageData = std::vector<unsigned char>;
        using Size = std::vector<unsigned>;
        using NameMap = std::map<unsigned, std::string>;

    private:
        ImageData copy_image_data(void *p_data);

        long get_byte_count() const;
        unsigned get_bytes_per_px() const;
        unsigned get_bits_per_px() const;
        unsigned get_component_count() const; // ParameterIdImageProcessingOutputFormat

        long get_byte_count_hw() const;
        unsigned get_bytes_per_px_hw() const;
        unsigned get_bits_per_px_hw() const;
        unsigned get_component_count_hw() const; // ParameterIdImageProcessingOutputFormat

        long get_px_count() const;

        unsigned get_bytes_per_component() const;
        unsigned extract_bits_per_component() const; // ParameterIdImageModeBits
        Size extract_size() const; // ParameterIdImageModeSize

        NameMap get_component_name_map() const;

    private:
        Camera *m_p_camera;
        ImageData m_data;

        static const NameMap M_S_RGBA_COMPONENT_NAMES;
        static const NameMap M_S_GRAY_COMPONENT_NAMES;
    };
}

#endif
