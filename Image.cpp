#include "Image.h"

#include "Parameters.h"

#include "dijsdk.h"
#include "parameterif.h"

#include <array>
#include <cassert>

namespace Prokyon {
    // public member functions
    Image::Image(CameraHandle camera) :
        m_handle{nullptr},
        m_p_data{nullptr},
        m_component_count{0},
        m_bits_per_channel{0},
        m_component_names{NameMap()},
        m_size{0, 0}
    {
        ImageHandle image_handle;
        void *p_raw_data = nullptr;
        DijSDK_GetImage(camera, &image_handle, &p_raw_data);

        auto format_id = get_format_id(image_handle);
        auto component_count = get_component_count(format_id);
        auto bits_per_channel = get_bits_per_channel(format_id);
        auto component_names = get_component_name_map(format_id);
        auto size = get_size(image_handle);

        m_handle = image_handle;
        m_p_data = static_cast<ImageBuffer>(p_raw_data);
        m_component_count = component_count;
        m_bits_per_channel = bits_per_channel;
        m_component_names = component_names;
        m_size = size;
    }

    Image::~Image() {
        DijSDK_ReleaseImage(m_handle);
        m_handle = nullptr;
        m_p_data = nullptr;
    }

    const unsigned char *Image::get_image_buffer() const {
        return m_p_data;
    }

    unsigned Image::get_number_of_components() const {
        assert(m_component_count > 0);
        return m_component_count;
    }

    std::string Image::get_component_name(unsigned component) const {
        assert(m_component_names.size() == m_component_count);
        assert(!m_component_names.empty());
        return m_component_names.at(component);
    }

    long Image::get_image_buffer_size() const {
        return static_cast<long>(get_image_width())
            * static_cast<long>(get_image_height())
            * static_cast<long>(get_image_bytes_per_pixel());
    }

    unsigned Image::get_image_width() const {
        assert(m_size.size() == 2);
        return m_size[0];
    }

    unsigned Image::get_image_height() const {
        assert(m_size.size() == 2);
        return m_size[1];
    }

    unsigned Image::get_image_bytes_per_pixel() const {
        return get_bit_depth()
            * get_number_of_components();
    }

    unsigned Image::get_bit_depth() const {
        assert(m_bits_per_channel == 8 || m_bits_per_channel == 16);
        return m_bits_per_channel;
    }

    // private static member functions
    DijSDK_EImageFormat Image::get_format_id(ImageHandle image) {
        assert(image != nullptr);
        auto p = get_numeric_parameter<int>(image, ParameterIdImageProcessingOutputFormat, 1);
        if (p.error) {
            // TODO handle error
        }
        assert(!p.value.empty());
        return static_cast<DijSDK_EImageFormat>(p.value[0]);
    }

    unsigned Image::get_component_count(DijSDK_EImageFormat format_id) {
        unsigned component_count = 0;
        switch (format_id) {
            // case DijSDK_EImageFormatGreyRaw16: UNSUPPORTED
            // case DijSDK_EImageFormatBayerRaw16: UNSUPPORTED
            case DijSDK_EImageFormatRGB888:
            case DijSDK_EImageFormatRGB161616:
                component_count = 3;
                break;
            case DijSDK_EImageFormatGrey8:
            case DijSDK_EImageFormatGrey16:
                component_count = 1;
                break;
            default:
                assert(false);
        }
        return component_count;
    }

    Image::NameMap Image::get_component_name_map(DijSDK_EImageFormat format_id) {
        auto map = NameMap();
        switch (format_id) {
            // case DijSDK_EImageFormatGreyRaw16: UNSUPPORTED
            // case DijSDK_EImageFormatBayerRaw16: UNSUPPORTED
            case DijSDK_EImageFormatRGB888:
            case DijSDK_EImageFormatRGB161616:
                map = M_S_RGB_COMPONENT_NAMES;
                break;
            case DijSDK_EImageFormatGrey8:
            case DijSDK_EImageFormatGrey16:
                map = M_S_GRAY_COMPONENT_NAMES;
                break;
            default:
                assert(false);
        }
        assert(!map.empty());
        return map;
    }

    unsigned Image::get_bits_per_channel(DijSDK_EImageFormat format_id) {
        unsigned bits_per_channel = 0;
        switch (format_id) {
            // case DijSDK_EImageFormatGreyRaw16: UNSUPPORTED
            // case DijSDK_EImageFormatBayerRaw16: UNSUPPORTED
            case DijSDK_EImageFormatRGB888:
            case DijSDK_EImageFormatGrey8:
                bits_per_channel = 8;
                break;
            case DijSDK_EImageFormatRGB161616:
            case DijSDK_EImageFormatGrey16:
                bits_per_channel = 16;
                break;
            default:
                assert(false);
        }
        return bits_per_channel;
    }

    std::vector<unsigned> Image::get_size(ImageHandle image) {
        assert(image != nullptr);
        auto p = get_numeric_parameter<int>(image, ParameterIdImageProcessingOutputFormat, 2);
        if (p.error) {
            // TODO handle error
        }
        assert(p.value.size() == 2);
        return to_unsigned(p.value);
    }

    // private static const members
    const Image::NameMap Image::M_S_RGB_COMPONENT_NAMES{
        {0, "red"},
        {1, "green"},
        {2, "blue"}
    };
    const Image::NameMap Image::M_S_GRAY_COMPONENT_NAMES{
        {0, "gray"}
    };
}