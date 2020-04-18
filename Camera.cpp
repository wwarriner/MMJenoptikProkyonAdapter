#include "Camera.h"

#include "dijsdk.h"
#include "dijsdkerror.h"

#include <cassert>
#include <sstream>

namespace Prokyon {
    Camera::Camera() :
        m_camera{nullptr},
        m_initialized{false},
        m_error{}{}

    // public
    bool Camera::initialize(const DijSDK_CameraKey *key, std::string name, std::string description) {
        if (m_initialized) {
            return true;
        }

        std::stringstream ss;

        // Initialize SDK
        auto result = DijSDK_Init(key, 1);
        if (!IS_OK(result)) {
            ss << "Error initializing " << name << std::endl;
            m_error = ss.str();
            return false;
        }

        // Get camera GUID
        // Only 1 camera supported at this time
        constexpr unsigned int EXPECTED_CAMERA_COUNT = 1;
        DijSDK_CamGuid cam_guid[EXPECTED_CAMERA_COUNT];
        unsigned int camera_count = EXPECTED_CAMERA_COUNT;
        // Always returns some guid for each camera requested.
        // Null camera appears to be "SynthCam::SynthCam::00000000"
        result = DijSDK_FindCameras(cam_guid, &camera_count);
        if (!IS_OK(result)) {
            ss << "Unable to create handle to camera." << std::endl;
            m_error = ss.str();
            return false;
        }

        // Open first camera
        result = DijSDK_OpenCamera(cam_guid[0], &m_camera);
        if (!IS_OK(result)) {
            ss << "Camera not valid." << std::endl;
            m_error = ss.str();
            return false;
        }

        assert(IS_OK(result));
        assert(m_camera != nullptr);
        m_initialized = true;
        m_error = std::string{};
        return true;
    }

    bool Camera::shutdown() {
        if (!m_initialized) {
            return true;
        }

        std::stringstream ss;

        // TODO check all image buffers are freed
        auto result = DijSDK_CloseCamera(m_camera);
        if (!IS_OK(result)) {
            ss << "Failed to close camera. ";
        }

        result = DijSDK_Exit();
        if (!IS_OK(result)) {
            ss << "Error shutting down DijSDK. ";
        }

        m_camera = nullptr;
        m_initialized = false;
        if (ss.str().empty()) {
            return true;
        }
        else {
            ss << std::endl;
            m_error = ss.str();
            return false;
        }
    }

    Camera::operator CameraHandle() const {
        assert(m_camera != nullptr);
        return m_camera;
    }

    Camera::operator CameraHandle() {
        assert(m_camera != nullptr);
        return m_camera;
    }

    const CameraHandle &Camera::operator*() const {
        assert(m_camera != nullptr);
        return m_camera;
    }

    CameraHandle &Camera::operator*() {
        assert(m_camera != nullptr);
        return m_camera;
    }

    std::string Camera::get_error() const {
        return m_error;
    }
}