#include "ProkyonCamera.h"

#include "Image.h"
#include "TestImage.h"
#include "TestRegionOfInterest.h"
#include "AcquisitionParameters.h"
#include "RegionOfInterest.h"
#include "Camera.h"

#include "MMDevice/ModuleInterface.h"
#include "MMDevice/MMDeviceConstants.h"
#include "dijsdk.h"
#include "dijsdkerror.h"
#include "parameterif.h"

#include <cassert>
#include <sstream>
#include <memory>
#include <algorithm> // debug

// ModuleInterface.h
// Required for initialization and DLL export

MODULE_API void InitializeModuleData() {
    RegisterDevice(Prokyon::ProkyonCamera::get_name(), MM::CameraDevice, Prokyon::ProkyonCamera::get_description());
}

MODULE_API MM::Device *CreateDevice(const char *name) {
    if (name == nullptr) {
        return nullptr;
    }
    else if (std::string{name} == Prokyon::ProkyonCamera::get_name()) {
        return new Prokyon::ProkyonCamera{};
    }
    else {
        return nullptr;
    }
}

MODULE_API void DeleteDevice(MM::Device *pDevice) {
    delete pDevice;
}

namespace Prokyon {
    // DeviceBase
    ProkyonCamera::ProkyonCamera() : CCameraBase<ProkyonCamera>(),
        m_p_camera{std::make_unique<Camera>()},
        m_p_image{std::make_unique<TestImage>()},
        m_p_acq_parameters{nullptr},
        m_p_roi{nullptr}
    {
    }

    int ProkyonCamera::Initialize() {
        auto success = m_p_camera->initialize(&M_S_KEY, M_S_CAMERA_NAME, M_S_CAMERA_DESCRIPTION);
        if (!success) {
            LogMessage(m_p_camera->get_error());
            return DEVICE_ERR;
        }
        else {
            auto test_image = TestImage();
            auto w = test_image.get_image_width();
            auto h = test_image.get_image_height();
            ROI roi{w - 1, h - 1, w, h};
            m_p_roi = std::make_unique<TestRegionOfInterest>(roi);
            m_p_acq_parameters = std::make_unique<AcquisitionParameters>(m_p_camera.get(), m_p_roi.get());
            return DEVICE_OK;
        }
    }

    int ProkyonCamera::Shutdown() {
        auto success = m_p_camera->shutdown();
        if (!success) {
            LogMessage(m_p_camera->get_error());
            return DEVICE_ERR;
        }
        else {
            m_p_roi.reset(nullptr);
            m_p_acq_parameters.reset(nullptr);
            return DEVICE_OK;
        }
    }

    void ProkyonCamera::GetName(char *name) const {
        CDeviceUtils::CopyLimitedString(name, M_S_CAMERA_NAME.c_str());
    }

    // CameraBase
    int ProkyonCamera::SnapImage() {
        // TODO
        return DEVICE_OK;
    }

    const unsigned char *ProkyonCamera::GetImageBuffer() {
        return m_p_image->get_image_buffer();
    }

    unsigned ProkyonCamera::GetNumberOfComponents() const {
        return m_p_image->get_number_of_components();
    }

    int ProkyonCamera::GetComponentName(unsigned component, char *name) {
        if (component > GetNumberOfComponents()) {
            return DEVICE_NONEXISTENT_CHANNEL;
        }
        else {
            auto out = m_p_image->get_component_name(component);
            CDeviceUtils::CopyLimitedString(name, out.c_str());
            return DEVICE_OK;
        }
    }

    long ProkyonCamera::GetImageBufferSize() const {
        return GetImageWidth() * GetImageHeight() * GetImageBytesPerPixel();
    }

    unsigned ProkyonCamera::GetImageWidth() const {
        return m_p_image->get_image_height();
    }

    unsigned ProkyonCamera::GetImageHeight() const {
        return m_p_image->get_image_width();
    }

    unsigned ProkyonCamera::GetImageBytesPerPixel() const {
        return m_p_image->get_image_bytes_per_pixel();
    }

    unsigned ProkyonCamera::GetBitDepth() const {
        return m_p_image->get_bit_depth();
    }

    double ProkyonCamera::GetPixelSizeUm() const {
        return m_p_image->get_pixel_size_um();
    }

    int ProkyonCamera::GetBinning() const {
        // TODO
        return 1;
    }

    int ProkyonCamera::SetBinning(int binSize) {
        // TODO
        return DEVICE_OK;
    }

    void ProkyonCamera::SetExposure(double exp_ms) {
        // TODO
        // no-op
    }

    double ProkyonCamera::GetExposure() const {
        // TODO
        return 0;
    }

    int ProkyonCamera::SetROI(unsigned x, unsigned y, unsigned xSize, unsigned ySize) {
        assert(m_p_roi != nullptr);
        m_p_roi->set({x, y, xSize, ySize});
        return DEVICE_OK;
    }

    int ProkyonCamera::GetROI(unsigned &x, unsigned &y, unsigned &xSize, unsigned &ySize) {
        assert(m_p_roi != nullptr);
        auto roi = m_p_roi->get();
        x = roi.at(X_ind);
        y = roi.at(Y_ind);
        xSize = roi.at(W_ind);
        ySize = roi.at(H_ind);
        return DEVICE_OK;
    }

    int ProkyonCamera::ClearROI() {
        assert(m_p_roi != nullptr);
        m_p_roi->clear();
        return DEVICE_OK;
    }

    int ProkyonCamera::StartSequenceAcquisition(long numImages, double interval_ms, bool stopOnOverflow) {
        // TODO
        return DEVICE_OK;
    }

    int ProkyonCamera::PrepareSequenceAcqusition() {
        // TODO
        return DEVICE_OK;
    }

    bool ProkyonCamera::IsCapturing() {
        // TODO
        return false;
    }

    int ProkyonCamera::IsExposureSequenceable(bool &isSequencable) const {
        // TODO what is this?
        isSequencable = false;
        return DEVICE_OK;
    }

    int ProkyonCamera::GetExposureSequenceMaxLength(long &nrEvents) const {
        // TODO what is this?
        bool sequenceable = false;
        IsExposureSequenceable(sequenceable);
        if (!sequenceable) {
            return DEVICE_UNSUPPORTED_COMMAND;
        }
        else {
            nrEvents = 1l;
            return DEVICE_OK;
        }
    }

    int ProkyonCamera::StartExposureSequence() {
        // TODO what is this?
        return DEVICE_UNSUPPORTED_COMMAND;
    }

    int ProkyonCamera::StopExposureSequence() {
        // TODO what is this?
        return DEVICE_UNSUPPORTED_COMMAND;
    }

    int ProkyonCamera::ClearExposureSequence() {
        // TODO what is this?
        return DEVICE_UNSUPPORTED_COMMAND;
    }

    int ProkyonCamera::AddToExposureSequence(double exposureTime_ms) {
        // TODO what is this ?
        return DEVICE_UNSUPPORTED_COMMAND;
    }

    int ProkyonCamera::SendExposureSequence() const {
        // TODO what is this?
        return DEVICE_UNSUPPORTED_COMMAND;
    }

    const char *ProkyonCamera::get_name() {
        return M_S_CAMERA_NAME.c_str();
    }

    const char *ProkyonCamera::get_description() {
        const unsigned int length = 128;
        char version[length];
        auto result = DijSDK_GetVersion(version, length);

        std::stringstream ss;
        ss << M_S_CAMERA_DESCRIPTION << std::endl;
        if (IS_OK(result)) {
            ss << " " << version;
        }
        return ss.str().c_str();
    }

    // private
    int ProkyonCamera::log_error(const char *func, const int line, const std::string &message) const {
        LogMessage(format_error(func, line, message), true);
        return DEVICE_ERR;
    }

    const DijSDK_CameraKey ProkyonCamera::M_S_KEY{"C941DD58617B5CA774Bf12B70452BF23"};
    const std::string ProkyonCamera::M_S_CAMERA_NAME{"Prokyon"};
    const std::string ProkyonCamera::M_S_CAMERA_DESCRIPTION{"Jenoptik Prokyon"};

    // debug

    void ProkyonCamera::LogProperties() const {
        std::vector<SDKParameter> parameters{
            {"ParameterIdImageCaptureExposureTimeUsec", static_cast<DijSDK_EParamId>(0x20000000)},
            {"ParameterIdImageCaptureGain", static_cast<DijSDK_EParamId>(0x30000001)},
            {"ParameterIdImageCaptureRoi", static_cast<DijSDK_EParamId>(0x20000002)},
            {"ParameterIdImageCaptureTriggerInputMode", static_cast<DijSDK_EParamId>(0x20000003)},
            {"ParameterIdImageCaptureTriggerOutputModeT1", static_cast<DijSDK_EParamId>(0x20000004)},
            {"ParameterIdImageCaptureTriggerOutputModeT2", static_cast<DijSDK_EParamId>(0x20000005)},
            {"ParameterIdImageCaptureTriggerOutputModeT3", static_cast<DijSDK_EParamId>(0x20000006)},
            {"ParameterIdImageCaptureFrameRate", static_cast<DijSDK_EParamId>(0x30000007)},
            {"ParameterIdSensorSize", static_cast<DijSDK_EParamId>(0x20000080)},
            {"ParameterIdSensorColorChannels", static_cast<DijSDK_EParamId>(0x20000081)},
            {"ParameterIdSensorRedOffset", static_cast<DijSDK_EParamId>(0x20000082)},
            {"ParameterIdSensorPixelSizeUm", static_cast<DijSDK_EParamId>(0x30000083)},
            {"ParameterIdSensorNumberOfBits", static_cast<DijSDK_EParamId>(0x20000084)},
            {"ParameterIdSensorFrequenciesMHz", static_cast<DijSDK_EParamId>(0x20000085)},
            {"ParameterIdSensorImageCounter", static_cast<DijSDK_EParamId>(0x20000087)},
            {"ParameterIdImageModeIndex", static_cast<DijSDK_EParamId>(0x20000100)},
            {"ParameterIdImageModeVirtualIndex", static_cast<DijSDK_EParamId>(0x20000101)},
            {"ParameterIdImageModeShadingIndex", static_cast<DijSDK_EParamId>(0x20000102)},
            {"ParameterIdImageModeSize", static_cast<DijSDK_EParamId>(0x20000103)},
            {"ParameterIdImageModeSubsampling", static_cast<DijSDK_EParamId>(0x20000104)},
            {"ParameterIdImageModeAveraging", static_cast<DijSDK_EParamId>(0x20000105)},
            {"ParameterIdImageModeSumming", static_cast<DijSDK_EParamId>(0x20000106)},
            {"ParameterIdImageModeBits", static_cast<DijSDK_EParamId>(0x20000107)},
            {"ParameterIdImageModePreferredAcqMode", static_cast<DijSDK_EParamId>(0x20000108)},
            {"ParameterIdImageModeScan", static_cast<DijSDK_EParamId>(0x20000109)},
            {"ParameterIdImageModeName", static_cast<DijSDK_EParamId>(0x4000010A)},
            {"ParameterIdImageModePixelSizeMicroMeter", static_cast<DijSDK_EParamId>(0x3000010B)},
            {"ParameterIdImageProcessingOutputFormat", static_cast<DijSDK_EParamId>(0x20000200)},
            {"ParameterIdImageProcessingWhiteBalance", static_cast<DijSDK_EParamId>(0x30000201)},
            {"ParameterIdImageProcessingWhiteBalanceMode", static_cast<DijSDK_EParamId>(0x20000202)},
            {"ParameterIdImageProcessingWhiteBalanceUpdateMode", static_cast<DijSDK_EParamId>(0x20000203)},
            {"ParameterIdImageProcessingWhiteBalanceRoi", static_cast<DijSDK_EParamId>(0x30000204)},
            {"ParameterIdImageProcessingGammaCorrection", static_cast<DijSDK_EParamId>(0x30000205)},
            {"ParameterIdImageProcessingContrast", static_cast<DijSDK_EParamId>(0x30000206)},
            {"ParameterIdImageProcessingSharpness", static_cast<DijSDK_EParamId>(0x30000207)},
            {"ParameterIdImageProcessingHighDynamicRange", static_cast<DijSDK_EParamId>(0x10000208)},
            {"ParameterIdImageProcessingHdrWeight", static_cast<DijSDK_EParamId>(0x30000209)},
            {"ParameterIdImageProcessingHdrColorGamma", static_cast<DijSDK_EParamId>(0x3000020A)},
            {"ParameterIdImageProcessingHdrSmoothFieldSize", static_cast<DijSDK_EParamId>(0x2000020B)},
            {"ParameterIdImageProcessingOrientation", static_cast<DijSDK_EParamId>(0x2000020C)},
            {"ParameterIdImageProcessingXyzWhite", static_cast<DijSDK_EParamId>(0x3000020D)},
            {"ParameterIdImageProcessingColorMatrixMode", static_cast<DijSDK_EParamId>(0x2000020E)},
            {"ParameterIdImageProcessingColorMatrix", static_cast<DijSDK_EParamId>(0x3000020F)},
            {"ParameterIdImageProcessingIllumination", static_cast<DijSDK_EParamId>(0x20000210)},
            {"ParameterIdImageProcessingColorBalance", static_cast<DijSDK_EParamId>(0x30000211)},
            {"ParameterIdImageProcessingColorBalKeepBrightness", static_cast<DijSDK_EParamId>(0x10000212)},
            {"ParameterIdImageProcessingSaturation", static_cast<DijSDK_EParamId>(0x30000213)},
            {"ParameterIdImageProcessingBlackBalance", static_cast<DijSDK_EParamId>(0x30000214)},
            {"ParameterIdImageProcessingBlackBalanceRoi", static_cast<DijSDK_EParamId>(0x30000215)},
            {"ParameterIdImageProcessingContNoiseFilterLevel", static_cast<DijSDK_EParamId>(0x20000216)},
            {"ParameterIdImageProcessingContNoiseFilterQuality", static_cast<DijSDK_EParamId>(0x20000217)},
            {"ParameterIdImageProcessingContNoiseFilterCtrlMode", static_cast<DijSDK_EParamId>(0x20000218)},
            {"ParameterIdImageProcessingRawBlackOffset16", static_cast<DijSDK_EParamId>(0x20000219)},
            {"ParameterIdImageProcessingColorSpace", static_cast<DijSDK_EParamId>(0x2000021A)},
            {"ParameterIdImageProcessingColorSkew", static_cast<DijSDK_EParamId>(0x3000021B)},
            {"ParameterIdImageProcessingWhiteShadingAvailable", static_cast<DijSDK_EParamId>(0x2000021E)},
            {"ParameterIdImageProcessingBlackShadingAvailable", static_cast<DijSDK_EParamId>(0x2000021F)},
            {"ParameterIdImageProcessingWhiteShadingEnable", static_cast<DijSDK_EParamId>(0x10000220)},
            {"ParameterIdImageProcessingBlackShadingEnable", static_cast<DijSDK_EParamId>(0x10000221)},
            {"ParameterIdImageProcessingImageType", static_cast<DijSDK_EParamId>(0x20000222)},
            {"ParameterIdImageProcessingHistogramRed", static_cast<DijSDK_EParamId>(0x20000223)},
            {"ParameterIdImageProcessingHistogramGreen", static_cast<DijSDK_EParamId>(0x20000224)},
            {"ParameterIdImageProcessingHistogramGreen2", static_cast<DijSDK_EParamId>(0x20000225)},
            {"ParameterIdImageProcessingHistogramBlue", static_cast<DijSDK_EParamId>(0x20000226)},
            {"ParameterIdImageProcessingHistogramGrey", static_cast<DijSDK_EParamId>(0x20000227)},
            {"ParameterIdImageProcessingHistogramRoi", static_cast<DijSDK_EParamId>(0x30000228)},
            {"ParameterIdImageProcessingCutHistogramBorder", static_cast<DijSDK_EParamId>(0x10000229)},
            {"ParameterIdImageProcessingSecondaryError", static_cast<DijSDK_EParamId>(0x20000237)},
            {"ParameterIdImageProcessingProcessorCores", static_cast<DijSDK_EParamId>(0x20000238)},
            {"ParameterIdImageProcessingAttachRawImages", static_cast<DijSDK_EParamId>(0x10000267)},
            {"ParameterIdImageProcessingOutputFifoSize", static_cast<DijSDK_EParamId>(0x20000268)},
            {"ParameterIdCameraFeaturesCooling", static_cast<DijSDK_EParamId>(0x20000280)},
            {"ParameterIdCameraFeaturesVentilation", static_cast<DijSDK_EParamId>(0x10000281)},
            {"ParameterIdCameraFeaturesTriggerOutputPin", static_cast<DijSDK_EParamId>(0x10000282)},
            {"ParameterIdCameraFeaturesTriggerInputPin", static_cast<DijSDK_EParamId>(0x10000283)},
            {"ParameterIdCameraFeaturesIlluminationIntensity", static_cast<DijSDK_EParamId>(0x20000290)},
            {"ParameterIdCameraFeaturesZPosition", static_cast<DijSDK_EParamId>(0x200002A0)},
            {"ParameterIdExposureControlMode", static_cast<DijSDK_EParamId>(0x20000300)},
            {"ParameterIdExposureControlAlgorithm", static_cast<DijSDK_EParamId>(0x20000302)},
            {"ParameterIdExposureControlRoi", static_cast<DijSDK_EParamId>(0x30000303)},
            {"ParameterIdExposureControlBrightnessPercentage", static_cast<DijSDK_EParamId>(0x20000304)},
            {"ParameterIdExposureControlExposureLimits", static_cast<DijSDK_EParamId>(0x20000305)},
            {"ParameterIdExposureControlMaxExposure", static_cast<DijSDK_EParamId>(0x20000305)},
            {"ParameterIdExposureControlGainLimits", static_cast<DijSDK_EParamId>(0x30000306)},
            {"ParameterIdExposureControlMaxGain", static_cast<DijSDK_EParamId>(0x30000306)},
            {"ParameterIdExposureControlMaxOePercentage", static_cast<DijSDK_EParamId>(0x2000030A)},
            {"ParameterIdExposureControlStatus", static_cast<DijSDK_EParamId>(0x2000030F)},
            {"ParameterIdGlobalSettingsCameraName", static_cast<DijSDK_EParamId>(0x40000380)},
            {"ParameterIdGlobalSettingsCameraSerialNumber", static_cast<DijSDK_EParamId>(0x40000381)},
            {"ParameterIdGlobalSettingsOpenCameraWarning", static_cast<DijSDK_EParamId>(0x20000382)},
            {"ParameterIdGlobalSettingsCameraBoardNumber", static_cast<DijSDK_EParamId>(0x40000386)},
            {"ParameterIdGlobalSettingsApiLoggerFileName", static_cast<DijSDK_EParamId>(0x4000038A)},
            {"ParameterIdGlobalSettingsErrorLoggerFileName", static_cast<DijSDK_EParamId>(0x4000038B)},
            {"ParameterIdFactorySettingsDefaultFirmwareUpdateFile", static_cast<DijSDK_EParamId>(0x40000400)},
            {"ParameterIdCustomerSettingsCustomerName", static_cast<DijSDK_EParamId>(0x40000500)},
            {"ParameterIdCustomerSettingsUserValue1", static_cast<DijSDK_EParamId>(0x40000501)},
            {"ParameterIdCustomerSettingsUserValue2", static_cast<DijSDK_EParamId>(0x40000502)},
            {"ParameterIdCustomerSettingsUserValue3", static_cast<DijSDK_EParamId>(0x40000503)},
            {"ParameterIdCustomerSettingsUserValue4", static_cast<DijSDK_EParamId>(0x40000504)},
            {"ParameterIdCustomerSettingsUserValue5", static_cast<DijSDK_EParamId>(0x40000505)},
            {"ParameterIdCustomerSettingsUserValue6", static_cast<DijSDK_EParamId>(0x40000506)},
            {"ParameterIdCustomerSettingsUserValue7", static_cast<DijSDK_EParamId>(0x40000507)},
            {"ParameterIdCustomerSettingsUserValue8", static_cast<DijSDK_EParamId>(0x40000508)},
            {"ParameterIdCustomerSettingsUserValue9", static_cast<DijSDK_EParamId>(0x40000509)},
            {"ParameterIdCustomerSettingsUserValue10", static_cast<DijSDK_EParamId>(0x4000050A)},
            {"ParameterIdCustomerSettingsUserValue11", static_cast<DijSDK_EParamId>(0x4000050B)},
            {"ParameterIdCustomerSettingsUserValue12", static_cast<DijSDK_EParamId>(0x4000050C)},
            {"ParameterIdCustomerSettingsUserValue13", static_cast<DijSDK_EParamId>(0x4000050D)},
            {"ParameterIdCustomerSettingsUserValue14", static_cast<DijSDK_EParamId>(0x4000050E)},
            {"ParameterIdCustomerSettingsUserValue15", static_cast<DijSDK_EParamId>(0x4000050F)},
            {"ParameterIdCustomerSettingsUserValue16", static_cast<DijSDK_EParamId>(0x40000510)},
            {"ParameterIdCustomerSettingsSdkEnabledFeatures", static_cast<DijSDK_EParamId>(0x40000511)},
            {"ParameterIdCustomerSettingsAppEnabledFeatures", static_cast<DijSDK_EParamId>(0x40000512)},
            {"ParameterIdEndMarker", static_cast<DijSDK_EParamId>(0x7FFFFFFF)},
        };
        for (const auto p : parameters) {
            LogProperty(p);
        }
    }

    void ProkyonCamera::LogProperty(const SDKParameter parameter) const {
        error_t out = -1024;
        if (m_p_camera != nullptr) {
            out = DijSDK_HasParameter(m_p_camera.get(), parameter.id);
        }

        std::stringstream ss;
        ss << parameter.name << " (" << parameter.id << "): " << out;
        LogMessage(ss.str(), true);
    }
} // namespace Prokyon