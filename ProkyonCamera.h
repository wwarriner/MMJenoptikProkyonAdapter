#pragma once

#ifndef PROKYONCAMERA_H_
#define PROKYONCAMERA_H_

#include "MMDevice/DeviceBase.h"

#include "CommonDef.h"
#include "Parameters.h"

#include <array>
#include <memory>
#include <string>

namespace Prokyon {
    class Camera;
    class Image;
    class RegionOfInterest;
    class AcquisitionParameters;

    class ProkyonCamera : public CCameraBase<ProkyonCamera> {
        using Camera = ::Prokyon::Camera;

    public:
        ProkyonCamera();
        //~ProkyonCamera();

        // device
        int Initialize(); // done
        int Shutdown(); // done
        void GetName(char *name) const; // done

        //debug
        int GetProperty(const char *name, char *value) const;
        int SetProperty(const char *name, const char *value);

        // camera
        int SnapImage();
        const unsigned char *GetImageBuffer();
        unsigned GetNumberOfComponents() const;
        int GetComponentName(unsigned component, char *name);
        long GetImageBufferSize() const;
        unsigned GetImageWidth() const;
        unsigned GetImageHeight() const;
        unsigned GetImageBytesPerPixel() const;
        unsigned GetBitDepth() const;
        double GetPixelSizeUm() const;
        int GetBinning() const;
        int SetBinning(int binSize);
        void SetExposure(double exp_ms);
        double GetExposure() const;
        int SetROI(unsigned x, unsigned y, unsigned xSize, unsigned ySize);
        int GetROI(unsigned &x, unsigned &y, unsigned &xSize, unsigned &ySize);
        int ClearROI();
        int IsExposureSequenceable(bool &isSequenceable) const;

    public:
        int update_mapped_numeric_scalar_property(MM::PropertyBase *p_prop, MM::ActionType type);
        // special case for image mode index and virtual image mode index
        int update_image_mode(MM::PropertyBase *p_prop, MM::ActionType type);

    public:
        static const char *get_name(); // done
        static const char *get_description(); // done

    private:
        int log_error(const char *func, const int line, const std::string &message = "") const;

        std::unique_ptr<Camera> m_p_camera;
        std::unique_ptr<Image> m_p_image;
        std::unique_ptr<AcquisitionParameters> m_p_acq_parameters;
        std::unique_ptr<RegionOfInterest> m_p_roi;

        std::map<std::string, MappedNumericScalarProperty<int>> m_mapped_int_scalar_properties;

        static const DijSDK_CameraKey M_S_KEY;
        static const std::string M_S_CAMERA_NAME;
        static const std::string M_S_CAMERA_DESCRIPTION;
        static const std::string M_S_IMAGE_MODE_NAME;
        static const std::string M_S_VIRTUAL_IMAGE_MODE_NAME;
        static const std::vector<unsigned char> M_S_TEST_IMAGE;

        // debug
        struct SDKParameter {
            std::string name;
            DijSDK_EParamId id;
        };
        /*
                    Returns E_FAIL if camera handle not found
                    */
        void LogProperties() const;
        /*
                    Returns E_OK if supported, E_DIJSDK_PARAMETER_NOT_AVAILABLE_ if not supported, E_FAIL if camera unavailable
                    */
        void LogProperty(const SDKParameter parameter) const;
    };
} // namespace Prokyon

#endif