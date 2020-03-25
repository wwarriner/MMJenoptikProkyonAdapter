#pragma once

#include "ImageInterface.h"

#include <map>
#include <string>
#include <vector>

typedef void *DijSDK_Handle;
enum DijSDK_EImageFormat : int;

namespace Prokyon {
    class Image : ImageInterface {
    public:
        Image(DijSDK_Handle image_handle);
        ~Image();

        virtual const unsigned char *get_image_buffer() const;
        virtual unsigned get_number_of_components() const;
        virtual std::string get_component_name(unsigned component) const;
        virtual long get_image_buffer_size() const;
        virtual unsigned get_image_width() const;
        virtual unsigned get_image_height() const;
        virtual unsigned get_image_bytes_per_pixel() const;
        virtual unsigned get_bit_depth() const;

    private:
        using NameMap = std::map<unsigned, std::string>;

    private:
        static DijSDK_EImageFormat get_format_id(DijSDK_Handle image);
        static unsigned get_component_count(DijSDK_EImageFormat format_id);
        static NameMap get_component_name_map(DijSDK_EImageFormat format_id);
        static unsigned get_bits_per_channel(DijSDK_EImageFormat format_id);
        static std::vector<unsigned> get_size(DijSDK_Handle image);

    private:
        DijSDK_Handle m_handle;
        unsigned char *m_p_data;
        unsigned m_component_count;
        unsigned m_bits_per_channel;
        NameMap m_component_names;
        std::vector<unsigned> m_size;

        static const NameMap M_S_RGB_COMPONENT_NAMES;
        static const NameMap M_S_GRAY_COMPONENT_NAMES;
    };
}