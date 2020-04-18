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
        m_size{0, 0},
        m_pixel_size_um{0.0}
    {
        ImageHandle image_handle;
        void *p_raw_data = nullptr;
        DijSDK_GetImage(camera, &image_handle, &p_raw_data);

        auto component_count = get_component_count(image_handle);
        auto component_names = get_component_name_map(component_count);
        auto bits_per_channel = get_bits_per_channel(image_handle);
        auto size = get_size(image_handle);
        auto pixel_size_um = get_pixel_size_um(image_handle);

        m_handle = image_handle;
        m_p_data = static_cast<ImageBuffer>(p_raw_data);
        m_component_count = component_count;
        m_bits_per_channel = bits_per_channel;
        m_component_names = component_names;
        m_size = size;
        m_pixel_size_um = pixel_size_um;
    }

    Image::~Image() {
        DijSDK_ReleaseImage(m_handle);
        m_handle = nullptr;
        m_p_data = nullptr;
    }

    const unsigned char *Image::get_image_buffer() {
        return static_cast<const Image *>(this)->get_image_buffer();
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

    double Image::get_pixel_size_um() const {
        assert(0.0 < m_pixel_size_um);
        return m_pixel_size_um;
    }

    unsigned Image::get_component_count(ImageHandle image) {
        assert(image != nullptr);
        auto p = get_numeric_parameter<int>(image, ParameterIdSensorColorChannels, 1);
        if (p.error) {
            // TODO handle error
        }
        auto count = to_unsigned(p.value.at(0));
        assert(count == 1 || count == 4);
        return count;
    }

    Image::NameMap Image::get_component_name_map(unsigned count) {
        assert(count == 1 || count == 4);
        NameMap map;
        switch (count) {
            case 1:
                map = M_S_GRAY_COMPONENT_NAMES;
                break;
            case 4:
                map = M_S_RGBA_COMPONENT_NAMES;
                break;
            default:
                assert(false);
        }
        assert(map.size() == count);
        return map;
    }

    unsigned Image::get_bits_per_channel(ImageHandle image) {
        assert(image != nullptr);
        auto p = get_numeric_parameter<int>(image, ParameterIdImageModeBits, 1);
        if (p.error) {
            // TODO handle error
        }
        return to_unsigned(p.value.at(0));
    }

    std::vector<unsigned> Image::get_size(ImageHandle image) {
        assert(image != nullptr);
        unsigned COUNT = 2;
        auto p = get_numeric_parameter<int>(image, ParameterIdImageModeSize, COUNT);
        if (p.error) {
            // TODO handle error
        }
        return to_unsigned(p.value);
    }

    double Image::get_pixel_size_um(ImageHandle image) {
        assert(image != nullptr);
        unsigned COUNT = 1;
        auto p = get_numeric_parameter<double>(image, ParameterIdImageModePixelSizeMicroMeter, COUNT);
        if (p.error) {
            // TODO handle error
        }
        return p.value.at(0);
    }

    // private static const members
    const Image::NameMap Image::M_S_RGBA_COMPONENT_NAMES{
        {0, "red"},
        {1, "green"},
        {2, "blue"},
        {3, "alpha"}
    };
    const Image::NameMap Image::M_S_GRAY_COMPONENT_NAMES{
        {0, "gray"}
    };
}