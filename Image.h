#pragma once

#ifndef PROKYON_IMAGE_H_
#define PROKYON_IMAGE_H_

#include <array>
#include <exception>
#include <map>
#include <string>
#include <vector>

using DijSDK_Handle = void *;

namespace Prokyon {
    class Camera;
    class ProkyonException;

    using ImageHandle = DijSDK_Handle;
    using ImageBuffer = const unsigned char *;

    class Image {
    public:
        Image(Camera *p_camera);

        bool acquire(); // returns success
        bool update(); // returns success

        ImageBuffer get_image_buffer();
        ImageBuffer get_image_buffer() const;
        unsigned get_number_of_components() const;
        std::string get_component_name(unsigned component) const;
        long get_image_buffer_size() const;
        unsigned get_image_width() const;
        unsigned get_image_height() const;
        unsigned get_image_bytes_per_pixel() const;
        unsigned get_bit_depth() const;

        std::string to_string() const;

    private:
        using ImageData = std::vector<unsigned char>;
        using Size = std::array<unsigned, 2u>;
        using NameMap = std::map<unsigned, std::string>;

    private:
        void copy_image_data(void *p_data); // modifies m_data

        unsigned compute_bits_per_px(unsigned bits_per_component, unsigned component_count) const;
        long compute_byte_count(unsigned bytes_per_px, const Size &size) const;
        long compute_px_count(const Size &size) const;
        unsigned to_bytes(unsigned bits) const;

        unsigned extract_component_count() const; // throws ProkyonException, ParameterIdImageProcessingOutputFormat
        unsigned extract_component_count_hw() const; // throws ProkyonException, ParameterIdImageProcessingOutputFormat
        Size extract_size() const; // throws ProkyonException, ParameterIdImageModeSize
        unsigned extract_bits_per_component() const; // throws ProkyonException, ParameterIdImageModeBits

        const NameMap *select_component_name_map(unsigned component_count) const;

        const void update_impl(Size size, unsigned bits_per_component, const NameMap *const component_names);

    private:
        Camera *m_p_camera;
        ImageData m_data;
        Size m_image_size;
        unsigned m_bits_per_component;
        const NameMap *m_p_component_names;

        static const Size M_S_IMAGE_SIZE_DEFAULT;
        static const unsigned M_S_BITS_PER_COMPONENT_DEFAULT = 8u;

        static const NameMap M_S_RGBA_COMPONENT_NAMES;
        static const NameMap M_S_GRAY_COMPONENT_NAMES;
        static const long M_S_BUFFER_SIZE = 3000l * 4000l * 4l; // 3000 px * 4000 px * 4 bytes is max buffer size needed for hardware
        static const unsigned X_ind = 0u;
        static const unsigned Y_ind = 1u;
    };

    class ImageException : public std::exception {};
}

#endif
