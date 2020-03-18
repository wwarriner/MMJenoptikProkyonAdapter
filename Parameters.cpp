#include "Parameters.h"

#include "dijsdk.h"
#include "parameterif.h"

#include <algorithm>
#include <vector>

namespace Prokyon {
    Parameters::Parameters(DijSDK_Handle handle) : Parameters() {
        auto raw_available_formats = new int[];
        unsigned int format_count = 0;
        auto result = DijSDK_GetParameterSpec(handle, ParameterIdImageProcessingOutputFormat, nullptr, nullptr, nullptr, nullptr, raw_values, &value_count);
        std::vector<int> i_formats(raw_available_formats, raw_available_formats + format_count);
        delete[] raw_available_formats;
        if (!IS_OK(result)) {
            return;
        }
        std::vector<DijSDK_EImageFormat> available_formats;
        for (const auto format : i_formats) {
            available_formats.push_back(static_cast<DijSDK_EImageFormat>(format));
        }

        std::vector<DijSDK_EImageFormat> priorities{
            DijSDK_EImageFormatRGB161616,
            DijSDK_EImageFormatRGB888,
            DijSDK_EImageFormatGrey16,
            DijSDK_EImageFormatGrey8
        };
        Format pick = Format{0, 0, 0};
        for (const auto priority : M_S_FORMATS) {
            auto itr = std::find(available_formats.cbegin(), available_formats.cend(), priority.first);
            if (itr != available_formats.cend()) {
                pick = priority.second;
                break;
            }
        }

        const unsigned int dimension = 2;
        int raw_shape[dimension];
        result = DijSDK_GetIntParameter(handle, ParameterIdSensorSize, raw_shape, dimension);
        if (!IS_OK(result)) {
            return;
        }
        m_shape = Shape{raw_shape[0], raw_shape[1]};
        m_bytes_per_pixel = pick.bytes_per_pixel;
        m_bit_depth = pick.bit_depth;
        m_channels = pick.channels;
    }

    Parameters::Parameters() :m_shape{0, 0}, m_bytes_per_pixel{0}, m_bit_depth{0}, m_channels{0} {
    }

    unsigned Parameters::width() const {
        return m_shape[0];
    }

    unsigned Parameters::height() const {
        return m_shape[1];
    }

    unsigned Parameters::bytes_per_pixel() const {
        return m_bytes_per_pixel;
    }

    unsigned Parameters::bit_depth() const {
        return m_bit_depth;
    }

    unsigned Parameters::channels() const {
        return m_channels;
    }

    // private

    std::map<DijSDK_EImageFormat, Parameters::Format> Parameters::M_S_FORMATS{
        {DijSDK_EImageFormatRGB161616, Format{6, 16, 3}},
        {DijSDK_EImageFormatRGB888, Format{3, 8, 3}},
        {DijSDK_EImageFormatGrey16, Format{2, 16, 1}},
        {DijSDK_EImageFormatGrey8, Format{1, 8, 1}},
    };
}