#include "Image.h"

#include "Camera.h"
#include "Parameters.h"

#include "dijsdk.h"
#include "parameterif.h"

#include <array>
#include <cassert>

// TODO error checking
// all functions relying on HW access can possibly fail
// we need a way to report that gracefully
// the "right" way is probably to propagate exceptions up to "acquire"
// swallow them there and immediately return, and have a method reporting failure, e.g. ok()
// then for any other public function that relies on HW access NOW (e.g. get_number_of_components())
// return a stored value
// this requires storing these values as they are computed in some way

namespace Prokyon {
    // public member functions
    Image::Image(Camera *p_camera) :
        m_p_camera{p_camera},
        m_data(M_S_BUFFER_SIZE, 0),
        m_image_size{M_S_IMAGE_SIZE_DEFAULT},
        m_bits_per_component{M_S_BITS_PER_COMPONENT_DEFAULT},
        m_p_component_names{&M_S_GRAY_COMPONENT_NAMES}
    {}

    bool Image::acquire() {
        auto result = DijSDK_StartAcquisition(*m_p_camera);
        if (result != E_OK) { return false; }

        ImageHandle image_handle;
        void *p_raw_data = nullptr;
        result = DijSDK_GetImage(*m_p_camera, &image_handle, &p_raw_data);
        if (result != E_OK) { return false; }

        try {
            copy_image_data(p_raw_data);
        }
        catch (ProkyonException) {
            return false;
        }

        result = DijSDK_ReleaseImage(image_handle);
        if (result != E_OK) { return false; }

        result = DijSDK_AbortAcquisition(*m_p_camera);
        if (result != E_OK) { return false; }

        return true;
    }

    bool Image::update() {
        try {
            update_impl(
                extract_size(),
                extract_bits_per_component(),
                select_component_name_map(extract_component_count())
            );
        }
        catch (ProkyonException) {
            return false;
        }
        return true;
    }

    ImageBuffer Image::get_image_buffer() {
        return m_data.data();
    }

    ImageBuffer Image::get_image_buffer() const {
        return m_data.data();
    }

    unsigned Image::get_number_of_components() const {
        auto num = m_p_component_names->size();
        assert(0 < num && num <= 4);
        return static_cast<unsigned>(num);
    }

    std::string Image::get_component_name(unsigned component) const {
        assert(m_p_component_names != nullptr);
        return m_p_component_names->at(component);
    }

    long Image::get_image_buffer_size() const {
        return get_image_width() * get_image_height() * get_image_bytes_per_pixel();
    }

    unsigned Image::get_image_width() const {
        if (m_p_camera == nullptr) {
            return M_S_IMAGE_SIZE_DEFAULT[X_ind];
        }
        else {
            assert(0u < m_image_size[X_ind]);
            return m_image_size[X_ind];
        }
    }

    unsigned Image::get_image_height() const {
        if (m_p_camera == nullptr) {
            return M_S_IMAGE_SIZE_DEFAULT[Y_ind];
        }
        else {
            assert(0u < m_image_size[Y_ind]);
            return m_image_size[Y_ind];
        }
    }

    unsigned Image::get_image_bytes_per_pixel() const {
        return get_number_of_components() * to_bytes(get_bit_depth());
    }

    unsigned Image::get_bit_depth() const {
        if (m_p_camera == nullptr) {
            return M_S_BITS_PER_COMPONENT_DEFAULT;
        }
        else {
            assert(0u < m_bits_per_component);
            return m_bits_per_component;
        }
    }

    std::string Image::to_string() const {
        std::stringstream ss;
        ss << "Image buffer information:\n";
        ss << "  address: " << this << "\n";
        ss << "  underlying address: " << get_image_buffer() << "\n";
        ss << "  components: " << get_number_of_components() << "\n";
        ss << "  bit depth: " << get_bit_depth() << "\n";
        ss << "  size (px): " << get_image_width() << ", " << get_image_height() << "\n";
        ss << "  bytes per pixel: " << get_image_bytes_per_pixel() << "\n";
        ss << "  total bytes: " << get_image_buffer_size() << "\n";
        return ss.str();
    }

    // private
    void Image::copy_image_data(void *p_data) {
        assert(p_data != nullptr);

        auto size = extract_size();
        auto px = compute_px_count(size);

        auto component_count = extract_component_count();
        auto component_count_hw = extract_component_count_hw();
        assert(component_count_hw <= component_count);

        auto pointer = static_cast<unsigned char *>(p_data);
        auto bits_per_component = extract_bits_per_component();
        auto bytes_per_c = to_bytes(bits_per_component);
        unsigned long index = 0ul;
        if (component_count_hw < component_count) {
            // color modes except BGRA should hit this
            for (long p = 0; p < px; ++p) {
                for (unsigned c = 0; c < component_count; ++c) {
                    for (unsigned b = 0; b < bytes_per_c; ++b) {
                        if (c < component_count_hw) {
                            m_data[index] = *pointer;
                            pointer += sizeof(*pointer);
                        }
                        else {
                            // MM expects 4 components, camera has only 3
                            // so we have to fill alpha channel with fully opaque
                            // (except BGRA mode)
                            m_data[index] = static_cast<unsigned char>(0xffu);
                        }
                        ++index;
                    }
                }
            }
        }
        else {
            // grayscale modes and BGRA should hit this
            for (long p = 0; p < px; ++p) {
                for (unsigned c = 0; c < component_count; ++c) {
                    for (unsigned b = 0; b < bytes_per_c; ++b) {
                        m_data[index] = *pointer;
                        pointer += sizeof(*pointer);
                        ++index;
                    }
                }
            }
        }

        update_impl(size, bits_per_component, select_component_name_map(component_count));
    }

    unsigned Image::compute_bits_per_px(unsigned bits_per_component, unsigned component_count) const {
        return bits_per_component * component_count;
    }

    long Image::compute_byte_count(unsigned bytes_per_px, const Size &size) const {
        return bytes_per_px * compute_px_count(size);
    }

    unsigned Image::to_bytes(unsigned bits) const {
        assert(bits % 8 == 0);
        return bits / 8;
    }

    long Image::compute_px_count(const Size &size) const {
        return static_cast<long>(size[0]) * size[1];
    }

    unsigned Image::extract_component_count() const {
        if (m_p_camera == nullptr) { throw ProkyonException(); }
        auto p = get_numeric_parameter<int>(*m_p_camera, ParameterIdImageProcessingOutputFormat, 1);
        if (p.error) { throw ProkyonException(); }
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

    unsigned Image::extract_component_count_hw() const {
        if (m_p_camera == nullptr) { throw ProkyonException(); }
        auto p = get_numeric_parameter<int>(*m_p_camera, ParameterIdImageProcessingOutputFormat, 1);
        if (p.error) { throw ProkyonException(); }
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

    Image::Size Image::extract_size() const {
        if (m_p_camera == nullptr) { throw ProkyonException(); }
        unsigned COUNT = 2;
        auto p = get_numeric_parameter<int>(*m_p_camera, ParameterIdImageModeSize, COUNT);
        if (p.error) { throw ProkyonException(); }
        assert(p.value.size() == COUNT);
        assert(0 < p.value[0]);
        assert(0 < p.value[1]);
        auto out = to_unsigned(p.value);
        return Size{out[0], out[1]};
    }

    unsigned Image::extract_bits_per_component() const {
        if (m_p_camera == nullptr) { throw ProkyonException(); }
        auto p = get_numeric_parameter<int>(*m_p_camera, ParameterIdImageProcessingOutputFormat, 1);
        if (p.error) { throw ProkyonException(); }
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

    const Image::NameMap *Image::select_component_name_map(unsigned component_count) const {
        const NameMap *map = nullptr;
        switch (component_count) {
            case 1:
                map = &M_S_GRAY_COMPONENT_NAMES;
                break;
            case 4:
                map = &M_S_RGBA_COMPONENT_NAMES;
                break;
            default:
                assert(false);
        }
        return map;
    }

    const void Image::update_impl(Size size, unsigned bits_per_component, const NameMap *const component_names) {
        m_image_size = size;
        m_bits_per_component = bits_per_component;
        m_p_component_names = component_names;
    }

    // private static const members
    const Image::Size Image::M_S_IMAGE_SIZE_DEFAULT{1u, 1u};

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