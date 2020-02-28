#pragma once

#ifndef PROKYONCAMERA_H_
#define PROKYONCAMERA_H_

#include "MMDevice/DeviceBase.h"
#include "MMDevice/ImgBuffer.h"
#include "MMDevice/DeviceThreads.h"

class ProkyonCamera : public CCameraBase<ProkyonCamera>
{
public:
    ProkyonCamera();
    ~ProkyonCamera();

    // device
    int Initialize();
    int Shutdown();
    void GetName(char *name) const;

    // camera
    int SnapImage();
    const unsigned char *GetImageBuffer();
    const unsigned char *GetImageBuffer(unsigned channelNr);
    const unsigned int *GetImageBufferAsRGB32();
    unsigned GetNumberOfComponents() const;
    int GetComponentName(unsigned component, char *name);
    int unsigned GetNumberOfChannels() const;
    int GetChannelName(unsigned channel, char *name);
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
    void GetTags(char *serializedMetadata);
    void AddTag(const char *key, const char *deviceLabel, const char *value);
    void RemoveTag(const char *key);
    int IsExposureSequenceable(bool &isSequenceable) const;
    int GetExposureSequenceMaxLength(long &nrEvents) const;
    int StartExposureSequence();
    int StopExposureSequence();
    int ClearExposureSequence();
    int AddToExposureSequence(double exposureTime_ms);
    int SendExposureSequence() const;
};

#endif