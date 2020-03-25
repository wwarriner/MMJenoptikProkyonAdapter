#pragma once

#ifndef PROKYON_IMAGE_INTERFACE_H_
#define PROKYON_IMAGE_INTERFACE_H_

#include <string>

namespace Prokyon {
    class ImageInterface {
    public:
        virtual const unsigned char *GetImageBuffer() const = 0;
        virtual unsigned GetNumberOfComponents() const = 0;
        virtual std::string GetComponentName(unsigned component) const = 0;
        virtual long GetImageBufferSize() const = 0;
        virtual unsigned GetImageWidth() const = 0;
        virtual unsigned GetImageHeight() const = 0;
        virtual unsigned GetImageBytesPerPixel() const = 0;
        virtual unsigned GetBitDepth() const = 0;
        // get metadata??
    };
}

#endif