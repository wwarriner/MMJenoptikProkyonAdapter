#pragma once

#ifndef PROKYON_CAMERA_H
#define PROKYON_CAMERA_H

#include "CommonDef.h"

#include <string>

namespace Prokyon {
    class Camera {
    public:
        Camera();

        enum class Status : int {
            state_changed = 0,
            no_change_needed = 1,
            failure = 255,
        };

        Status initialize(const DijSDK_CameraKey *key, std::string name, std::string description, const int camera_index = 1);
        Status shutdown();

        bool is_ready() const;

        operator CameraHandle() const;
        operator CameraHandle();
        const CameraHandle &operator*() const;
        CameraHandle &operator*();

        std::string get_error() const;
        std::string get_guid() const;

    private:
        CameraHandle m_camera;
        bool m_initialized;
        std::string m_error;
        std::string m_guid;
    };
}

#endif