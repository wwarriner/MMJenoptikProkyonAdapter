#pragma once

#include <array>
#include <map>

typedef void *DijSDK_Handle;
typedef enum DijSDK_EImageFormat;

namespace Prokyon {
    class Parameters {
    public:
        Parameters(DijSDK_Handle handle);
        Parameters();

        bool is_valid() const;

        unsigned width() const;
        unsigned height() const;
        unsigned bytes_per_pixel() const;
        unsigned bit_depth() const;
        unsigned channels() const;

    private:
        using Shape = std::array<int, 2>;
        Shape m_shape;
        unsigned m_bytes_per_pixel;
        unsigned m_bit_depth;
        unsigned m_channels;

        struct Format {
            unsigned bytes_per_pixel;
            unsigned bit_depth;
            unsigned channels;
        };

        static std::map<DijSDK_EImageFormat, Format> M_S_FORMATS;
    };
}