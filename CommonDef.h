#pragma once

#ifndef PROKYON_COMMON_DEF_H
#define PROKYON_COMMON_DEF_H

#include <array>

// external forward declarations
enum DijSDK_EImageFormat : int;
enum DijSDK_EParamId : int;
using DijSDK_Handle = void *;
using DijSDK_CameraKey = char[33];
using DijSDK_Handle = void *;
using DijSDK_CamGuid = char[128];

namespace Prokyon {
    // typedefs
    using CameraHandle = DijSDK_Handle;
    using ImageHandle = DijSDK_Handle;
    using ImageBuffer = const unsigned char *;
    using ROI = std::array<unsigned int, 4>;
    using CameraID = DijSDK_CamGuid;

    // constants
    static const unsigned int X_ind = 0;
    static const unsigned int Y_ind = 1;
    static const unsigned int W_ind = 2;
    static const unsigned int H_ind = 3;

    // utilities
    std::string format_error(const char *func, int line, const std::string &message);
}

#endif