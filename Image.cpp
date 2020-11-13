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
        m_p_camera{p_camera},
        m_data(3000u * 4000u, 0) // exact size of sensor for Prokyon Gryphax
    {}

    void Image::acquire() {
        // fn
        auto result = DijSDK_StartAcquisition(*m_p_camera);
        if (result != E_OK) {
            assert(false);
            // todo
        }

        // fn
        ImageHandle image_handle;
        void *p_raw_data = nullptr;
        result = DijSDK_GetImage(*m_p_camera, &image_handle, &p_raw_data);
        if (result != E_OK) {
            assert(false);
            // todo
        }

        m_data = copy_image_data(p_raw_data);

        // fn
        result = DijSDK_ReleaseImage(image_handle);
        if (result != E_OK) {
            assert(false);
            // todo
        }

        // fn
        result = DijSDK_AbortAcquisition(*m_p_camera);
        if (result != E_OK) {
            assert(false);
            // todo
        }
    }

    ImageBuffer Image::get_image_buffer() {
        return m_data.data();
    }

    ImageBuffer Image::get_image_buffer() const {
        return m_data.data();
    }

    unsigned Image::get_number_of_components() const {
        return get_component_count();
    }

    std::string Image::get_component_name(unsigned component) const {
        return get_component_name_map().at(component);
    }

    long Image::get_image_buffer_size() const {
        return static_cast<long>(m_data.size());
    }

    unsigned Image::get_image_width() const {
        return extract_size()[0];
    }

    unsigned Image::get_image_height() const {
        return extract_size()[1];
    }

    unsigned Image::get_image_bytes_per_pixel() const {
        return get_bytes_per_px();
    }

    unsigned Image::get_bit_depth() const {
        return extract_bits_per_component();
    }

    // private
    Image::ImageData Image::copy_image_data(void *p_data) {
        assert(p_data != nullptr);

        auto px = get_px_count();
        long bytes = px * get_bytes_per_px();

        auto component_count = get_component_count();
        auto component_count_hw = get_component_count_hw();
        assert(component_count_hw <= component_count);

        auto pointer = static_cast<unsigned char *>(p_data);
        auto bytes_per_c = get_bytes_per_component();
        ImageData out;
        if (component_count_hw < component_count) {
            // color modes except BGRA should hit this
            out = ImageData(bytes, 0);
            for (long p = 0; p < px; ++p) {
                for (unsigned c = 0; c < component_count; ++c) {
                    for (unsigned b = 0; b < bytes_per_c; ++b) {
                        auto index = p * component_count * bytes_per_c + c * bytes_per_c + b;
                        if (c < component_count_hw) {
                            out[index] = *pointer;
                            pointer += sizeof(*pointer);
                        }
                        else {
                            // MM expects 4 components, camera has only 3
                            // so we have to fill alpha channel with fully opaque
                            // (except BGRA mode)
                            out[index] = static_cast<unsigned char>(0xffu);
                        }
                    }
                }
            }
        }
        else {
            // grayscale modes and BGRA should hit this
            out = ImageData(pointer, pointer + (sizeof(*pointer) * bytes));
        }
        assert(out.size() == bytes);
        return out;
    }

    long Image::get_byte_count() const {
        return get_bytes_per_px() * get_px_count();
    }

    unsigned Image::get_bytes_per_px() const {
        return get_bits_per_px() / 8;
    }

    unsigned Image::get_bits_per_px() const {
        unsigned out = extract_bits_per_component() * get_component_count();
        return out;
    }

    unsigned Image::get_component_count() const {
        if (m_p_camera == nullptr) {
            return 1;
        }

        auto p = get_numeric_parameter<int>(*m_p_camera, ParameterIdImageProcessingOutputFormat, 1);
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

    long Image::get_byte_count_hw() const {
        return get_bytes_per_px_hw() * get_px_count();
    }

    unsigned Image::get_bytes_per_px_hw() const {
        return get_bits_per_px_hw() / 8;
    }

    unsigned Image::get_bits_per_px_hw() const {
        unsigned out = extract_bits_per_component() * get_component_count_hw();
        return out;
    }

    unsigned Image::get_component_count_hw() const {
        if (m_p_camera == nullptr) {
            return 1;
        }

        auto p = get_numeric_parameter<int>(*m_p_camera, ParameterIdImageProcessingOutputFormat, 1);
        if (p.error) {
            // TODO handle error
            assert(false);
        }
        auto entry = to_unsigned(p.value.at(0));

        unsigned count = 0;
        switch (entry) {
            case (DijSDK_EImageFormatNotSpecified):
                assert(false);
                break;
            case (DijSDK_EImageFormatBayerRaw16):
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
                count = 3;
                break;
            case (DijSDK_EImageFormatBGR888A):
                count = 4;
                break;
            default:
                assert(false);
        }
        assert(count == 1 || count == 3 || count == 4);
        return count;
    }

    long Image::get_px_count() const {
        Size size = extract_size();
        return static_cast<long>(size[0]) * size[1];
    }

    unsigned Image::get_bytes_per_component() const {
        return extract_bits_per_component() / 8;
    }

    unsigned Image::extract_bits_per_component() const {
        if (m_p_camera == nullptr) {
            return 8;
        }

        auto p = get_numeric_parameter<int>(*m_p_camera, ParameterIdImageProcessingOutputFormat, 1);
        if (p.error) {
            // TODO handle error
            assert(false);
        }
        auto entry = to_unsigned(p.value.at(0));

        unsigned bits = 0;
        switch (entry) {
            case (DijSDK_EImageFormatNotSpecified):
                assert(false);
                break;
            case (DijSDK_EImageFormatBayerRaw16):
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
        assert(bits != 0 && bits % 8 == 0);
        return bits;
    }

    Image::Size Image::extract_size() const {
        if (m_p_camera == nullptr) {
            return {1, 1};
        }

        unsigned COUNT = 2;
        auto p = get_numeric_parameter<int>(*m_p_camera, ParameterIdImageModeSize, COUNT);
        if (p.error) {
            // TODO handle error
            assert(false);
        }
        assert(p.value.size() == COUNT);
        assert(0 < p.value[0]);
        assert(0 < p.value[1]);
        return to_unsigned(p.value);
    }

    Image::NameMap Image::get_component_name_map() const {
        NameMap map;
        switch (get_component_count()) {
            case 1:
                map = M_S_GRAY_COMPONENT_NAMES;
                break;
            case 4:
                map = M_S_RGBA_COMPONENT_NAMES;
                break;
            default:
                assert(false);
        }
        return map;
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