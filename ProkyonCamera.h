#pragma once

#ifndef PROKYONCAMERA_H_
#define PROKYONCAMERA_H_

#include "MMDevice/DeviceBase.h"
#include "MMDevice/DeviceThreads.h"

#include "CommonDef.h"
#include "ImageInterface.h"
#include "Parameters.h"

#include <array>
#include <memory>
#include <string>

namespace Prokyon {
    class Camera;
    class ImageInterface;
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

        // camera
        int SnapImage();
        const unsigned char *GetImageBuffer();
        // const unsigned char *GetImageBuffer(unsigned channelNr); // DEFINED IN DeviceBase.h
        // const unsigned int *GetImageBufferAsRGB32(); // DEFINED IN DeviceBase.h
        unsigned GetNumberOfComponents() const;
        int GetComponentName(unsigned component, char *name);
        // unsigned GetNumberOfChannels() const; // DEFINED IN DeviceBase.h
        // int GetChannelName(unsigned channel, char *name); // DEFINED IN DeviceBase.h
        long GetImageBufferSize() const;
        unsigned GetImageWidth() const;
        unsigned GetImageHeight() const;
        unsigned GetImageBytesPerPixel() const;
        unsigned GetBitDepth() const;
        // double GetPixelSizeUm() const; // DEFINED IN DeviceBase.h
        int GetBinning() const;
        int SetBinning(int binSize);
        void SetExposure(double exp_ms);
        double GetExposure() const;
        int SetROI(unsigned x, unsigned y, unsigned xSize, unsigned ySize);
        int GetROI(unsigned &x, unsigned &y, unsigned &xSize, unsigned &ySize);
        int ClearROI();
        bool SupportsMultiROI();
        bool IsMultiROISet();
        int GetMultiROICount(unsigned &count);
        int SetMultiROI(const unsigned *xs, const unsigned *ys, const unsigned *widths, const unsigned *heights, unsigned numROIs);
        int GetMultiROI(unsigned *xs, unsigned *ys, unsigned *widths, unsigned *heights, unsigned *length);
        int StartSequenceAcquisition(long numImages, double interval_ms, bool stopOnOverflow);
        int StartSequenceAcquisition(double interval_ms);
        int StopSequenceAcquisition();
        int PrepareSequenceAcqusition();
        bool IsCapturing();
        // void GetTags(char *serializedMetadata); // DEFINED IN DeviceBase.h
        // void AddTag(const char *key, const char *deviceLabel, const char *value); // DEFINED IN DeviceBase.h
        // void RemoveTag(const char *key); // DEFINED IN DeviceBase.h
        int IsExposureSequenceable(bool &isSequenceable) const;
        int GetExposureSequenceMaxLength(long &nrEvents) const;
        int StartExposureSequence();
        int StopExposureSequence();
        int ClearExposureSequence();
        int AddToExposureSequence(double exposureTime_ms);
        int SendExposureSequence() const;

    public:
        static const char *get_name(); // done
        static const char *get_description(); // done

    private:
        int log_error(const char *func, const int line, const std::string &message = "") const;

        std::unique_ptr<Camera> m_p_camera;
        std::unique_ptr<ImageInterface> m_p_image;

        static const DijSDK_CameraKey M_S_KEY;
        static const std::string M_S_CAMERA_NAME;
        static const std::string M_S_CAMERA_DESCRIPTION;
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
}

#endif