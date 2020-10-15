#include "NullImage.h"

namespace Prokyon {
    ImageBuffer NullImage::get_image_buffer() {
        return nullptr;
    }

    ImageBuffer NullImage::get_image_buffer() const {
        return nullptr;
    }

    unsigned NullImage::get_number_of_components() const {
        return 1;
    }

    std::string NullImage::get_component_name(unsigned component) const {
        return "";
    }

    long NullImage::get_image_buffer_size() const {
        return 1;
    }

    unsigned NullImage::get_image_width() const {
        return 1;
    }

    unsigned NullImage::get_image_height() const {
        return 1;
    }

    unsigned NullImage::get_image_bytes_per_pixel() const {
        return 1;
    }

    unsigned NullImage::get_bit_depth() const {
        return 8;
    }
}