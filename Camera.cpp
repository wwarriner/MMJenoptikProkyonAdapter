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
    Camera::Status Camera::initialize(const DijSDK_CameraKey *key, std::string name, std::string description, const int camera_index) {
        if (m_initialized) {
            return Status::no_change_needed;
        }

        std::stringstream ss;

        // Initialize SDK
        auto result = DijSDK_Init(key, 1);
        if (!IS_OK(result)) {
            ss << "Error initializing " << name << std::endl;
            m_error = ss.str();
            return Status::failure;
        }

        // Get camera GUID
        // Only 1 camera supported at this time
        constexpr unsigned int EXPECTED_CAMERA_COUNT = 2;
        DijSDK_CamGuid cam_guid[EXPECTED_CAMERA_COUNT];
        unsigned int camera_count = EXPECTED_CAMERA_COUNT;
        // Always returns some guid for each camera requested.
        // Null camera appears to be "SynthCam::SynthCam::00000000"
        result = DijSDK_FindCameras(cam_guid, &camera_count);
        if (!IS_OK(result)) {
            ss << "Unable to create handle to camera." << std::endl;
            m_error = ss.str();
            return Status::failure;
        }
        const auto selected_guid = cam_guid[camera_index];

        // Open first camera
        result = DijSDK_OpenCamera(selected_guid, &m_camera);
        if (!IS_OK(result)) {
            ss << "Camera not valid." << std::endl;
            m_error = ss.str();
            return Status::failure;
        }

        assert(IS_OK(result));
        assert(m_camera != nullptr);
        m_initialized = true;
        m_error = std::string{};
        m_guid = std::string(selected_guid);
        return Status::state_changed;
    }

    Camera::Status Camera::shutdown() {
        if (!m_initialized) {
            return Status::no_change_needed;
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
            return Status::state_changed;
        }
        else {
            ss << std::endl;
            m_error = ss.str();
            return Status::failure;
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

    std::string Camera::get_guid() const {
        return m_guid;
    }
}