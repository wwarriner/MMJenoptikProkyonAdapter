#include "Image.h"

#include "Camera.h"
#include "Parameters.h"

#include "dijsdk.h"
#include "parameterif.h"

#include <array>
#include <cassert>

namespace Prokyon {
    // public member functions
    Image::Image(Camera *p_camera) :
        m_component_count{0},
        m_component_names{NameMap()},
        m_bits_per_channel{0},
        m_bits_per_pixel{0},
        m_bytes_per_pixel{0},
        m_size{0, 0},
        m_bytes{0},
        m_data{}
    {
        ImageHandle image_handle;
        void *p_raw_data = nullptr;
        auto result = DijSDK_GetImage(*p_camera, &image_handle, &p_raw_data);
        if (result != E_OK) {
            assert(false);
            // todo
        }

        auto component_count = extract_component_count(image_handle);
        auto component_names = build_component_name_map(component_count);
        auto bits_per_channel = extract_bits_per_channel(image_handle);
        auto bits_per_pixel = compute_bits_per_pixel(component_count, bits_per_channel);
        auto bytes_per_pixel = compute_bytes_per_pixel(bits_per_pixel);
        auto size = extract_size(image_handle);
        auto bytes = compute_image_bytes(size, bytes_per_pixel);
        auto data = copy_image_data(p_raw_data, bytes);

        result = DijSDK_ReleaseImage(image_handle);
        if (result != E_OK) {
            assert(false);
            // todo
        }

        m_component_count = component_count;
        m_component_names = component_names;
        m_bits_per_channel = bits_per_channel;
        m_bits_per_pixel = bits_per_pixel;
        m_bytes_per_pixel = bytes_per_pixel;
        m_size = size;
        m_bytes = bytes;
        m_data = data;
    }

    ImageBuffer Image::get_image_buffer() {
        return m_data.data();
    }

    ImageBuffer Image::get_image_buffer() const {
        return m_data.data();
    }

    unsigned Image::get_number_of_components() const {
        return m_component_count;
    }

    std::string Image::get_component_name(unsigned component) const {
        return m_component_names.at(component);
    }

    long Image::get_image_buffer_size() const {
        return m_bytes;
    }

    unsigned Image::get_image_width() const {
        return m_size[0];
    }

    unsigned Image::get_image_height() const {
        return m_size[1];
    }

    unsigned Image::get_image_bytes_per_pixel() const {
        return m_bytes_per_pixel;
    }

    unsigned Image::get_bit_depth() const {
        return m_bits_per_channel;
    }

    // private

    unsigned Image::extract_component_count(ImageHandle image) {
        assert(image != nullptr);
        auto p = get_numeric_parameter<int>(image, ParameterIdImageProcessingOutputFormat, 1);
        if (p.error) {
            // TODO handle error
            assert(false);
        }
        auto entry = to_unsigned(p.value.at(0));
        unsigned count = 0;
        switch (entry) {
            case (DijSDK_EImageFormatNotSpecified):
                // TODO handle this case
                assert(false);
                break;
            case (DijSDK_EImageFormatBayerRaw16):
                // TODO handle this case
                assert(false);
                break;
            case (DijSDK_EImageFormatGrey8):
            case (DijSDK_EImageFormatGrey16):
            case (DijSDK_EImageFormatGreyRaw16):
                count = 1;
                break;
            case (DijSDK_EImageFormatRGB888):
            case (DijSDK_EImageFormatRGB161616):
            case (DijSDK_EImageFormatBGR888):
            case (DijSDK_EImageFormatBGR888A):
                count = 4;
                break;
            default:
                assert(false);
        }
        assert(count == 1 || count == 4);
        return count;
    }

    Image::NameMap Image::build_component_name_map(unsigned component_count) {
        assert(component_count == 1 || component_count == 4);
        NameMap map;
        switch (component_count) {
            case 1:
                map = M_S_GRAY_COMPONENT_NAMES;
                break;
            case 4:
                map = M_S_RGBA_COMPONENT_NAMES;
                break;
            default:
                assert(false);
        }
        assert(map.size() == component_count);
        return map;
    }

    unsigned Image::extract_bits_per_channel(ImageHandle image) {
        assert(image != nullptr);
        auto p = get_numeric_parameter<int>(image, ParameterIdImageProcessingOutputFormat, 1);
        if (p.error) {
            // TODO handle error
            assert(false);
        }
        auto entry = to_unsigned(p.value.at(0));
        unsigned bits = 0;
        switch (entry) {
            case (DijSDK_EImageFormatNotSpecified):
                // TODO handle this case
                assert(false);
                break;
            case (DijSDK_EImageFormatBayerRaw16):
                // TODO handle this case
                assert(false);
                break;
            case (DijSDK_EImageFormatGrey8):
            case (DijSDK_EImageFormatRGB888):
            case (DijSDK_EImageFormatBGR888):
            case (DijSDK_EImageFormatBGR888A):
                bits = 8;
                break;
            case (DijSDK_EImageFormatGrey16):
            case (DijSDK_EImageFormatGreyRaw16):
            case (DijSDK_EImageFormatRGB161616):
                bits = 16;
                break;
            default:
                assert(false);
        }
        assert(bits == 8 || bits == 16);
        return bits;
    }

    unsigned Image::compute_bits_per_pixel(unsigned component_count, unsigned bits_per_pixel) {
        assert(component_count == 1 || component_count == 4);
        assert(bits_per_pixel == 8 || bits_per_pixel == 16);
        auto out = component_count * bits_per_pixel;
        assert(out == 8 || out == 16 || out == 32 || out == 64);
        return out;
    }

    unsigned Image::compute_bytes_per_pixel(unsigned bits_per_pixel) {
        assert(bits_per_pixel == 8 || bits_per_pixel == 16);
        auto out = bits_per_pixel / 8;
        auto rem = bits_per_pixel % 8;
        assert(out == 1 || out == 2);
        assert(rem == 0);
        return out;
    }

    Image::Size Image::extract_size(ImageHandle image) {
        assert(image != nullptr);
        unsigned COUNT = 2;
        auto p = get_numeric_parameter<int>(image, ParameterIdImageModeSize, COUNT);
        if (p.error) {
            // TODO handle error
            assert(false);
        }
        assert(p.value.size() == COUNT);
        assert(0 < p.value[0]);
        assert(0 < p.value[1]);
        return to_unsigned(p.value);
    }

    long Image::compute_image_bytes(std::vector<unsigned> size, unsigned bytes_per_pixel) {
        assert(size.size() == 2);
        assert(0 < size[0]);
        assert(0 < size[1]);
        assert(bytes_per_pixel == 1 || bytes_per_pixel == 2);
        auto out = size[0] * size[1] * bytes_per_pixel;
        assert(0 < out);
        return out;
    }

    Image::ImageData Image::copy_image_data(void *p_data, long bytes) {
        assert(p_data != nullptr);
        assert(0 < bytes);
        auto pointer = static_cast<unsigned char *>(p_data);
        ImageData out(pointer, pointer + bytes);
        assert(out.size() == bytes);
        return out;
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