#pragma once

#ifndef PROKYON_CAMERA_H
#define PROKYON_CAMERA_H

#include "CommonDef.h"

namespace Prokyon {
    class Camera {
    public:
        Camera();

        bool initialize(const DijSDK_CameraKey *key, std::string name, std::string description);
        bool shutdown();

        operator CameraHandle() const;
        operator CameraHandle();
        const CameraHandle &operator*() const;
        CameraHandle &operator*();

        std::string get_error() const;

    private:
        CameraHandle m_camera;
        bool m_initialized;
        std::string m_error;
    };
}

#endif