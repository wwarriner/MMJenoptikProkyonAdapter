#include "ProkyonCamera.h"

#include "Image.h"
#include "TestImage.h"

#include "MMDevice/ModuleInterface.h"
#include "MMDevice/MMDeviceConstants.h"
#include "dijsdk.h"
#include "dijsdkerror.h"
#include "parameterif.h"

#include <cassert>
#include <sstream>
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

    ProkyonCamera::ProkyonCamera() :
        m_camera_handle{nullptr},
        m_initialized{false},
        m_p_image{std::make_unique<TestImage>()}
    {
    }

    int ProkyonCamera::Initialize() {
        int out = DEVICE_OK;
        if (!m_initialized) {
            out = create_handle();
            if (out == DEVICE_OK) {
                m_initialized = true;
            }
        }
        return out;
    }

    int ProkyonCamera::Shutdown() {
        int out = DEVICE_OK;
        if (m_initialized) {
            out = destroy_handle();
            if (out == DEVICE_OK) {
                m_initialized = false;
            }
        }
        return out;
    }

    void ProkyonCamera::GetName(char *name) const {
        CDeviceUtils::CopyLimitedString(name, M_S_CAMERA_NAME.c_str());
    }

    // CameraBase

    int ProkyonCamera::SnapImage() {
        LogMessage("snap");
        if (valid()) {
            // HACK TEST IMAGE FOR NOW
            m_p_image = std::make_unique<TestImage>();
            //auto result = DijSDK_StartAcquisition(cam());
            // TODO handle error
            //m_p_image = std::make_unique<Image>(cam());
        }
        else {
            m_p_image = std::make_unique<TestImage>();
        }
        return DEVICE_OK;
    }

    const unsigned char *ProkyonCamera::GetImageBuffer() {
        LogMessage("get buffer");
        return m_p_image->get_image_buffer();
    }

    unsigned ProkyonCamera::GetNumberOfComponents() const {
        LogMessage("get component count");
        return m_p_image->get_number_of_components();
    }

    int ProkyonCamera::GetComponentName(unsigned component, char *name) {
        LogMessage("get component name");
        if (component > GetNumberOfComponents()) {
            return DEVICE_NONEXISTENT_CHANNEL;
        }
        else {
            auto out = m_p_image->get_component_name(component);
            LogMessage(out.c_str());
            CDeviceUtils::CopyLimitedString(name, out.c_str());
            return DEVICE_OK;
        }
    }

    long ProkyonCamera::GetImageBufferSize() const {
        LogMessage("get size");
        return GetImageWidth() * GetImageHeight() * GetImageBytesPerPixel();
    }

    unsigned ProkyonCamera::GetImageWidth() const {
        LogMessage("get width");
        return m_p_image->get_image_height();
    }

    unsigned ProkyonCamera::GetImageHeight() const {
        LogMessage("get height");
        return m_p_image->get_image_width();
    }

    unsigned ProkyonCamera::GetImageBytesPerPixel() const {
        LogMessage("get bpp");
        return m_p_image->get_image_bytes_per_pixel();
    }

    unsigned ProkyonCamera::GetBitDepth() const {
        LogMessage("get bits");
        return m_p_image->get_bit_depth();
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
        // TODO
        return DEVICE_OK;
    }

    int ProkyonCamera::GetROI(unsigned &x, unsigned &y, unsigned &xSize, unsigned &ysize) {
        // TODO
        return DEVICE_OK;
    }

    int ProkyonCamera::ClearROI() {
        // TODO
        return DEVICE_OK;
    }

    bool ProkyonCamera::SupportsMultiROI() {
        // TODO
        return false;
    }

    bool ProkyonCamera::IsMultiROISet() {
        // TODO
        return false;
    }

    int ProkyonCamera::GetMultiROICount(unsigned &count) {
        // TODO
        return DEVICE_UNSUPPORTED_COMMAND;
    }

    int ProkyonCamera::SetMultiROI(const unsigned *xs, const unsigned *ys, const unsigned *widths, const unsigned *heights, unsigned numROIs) {
        // TODO
        return DEVICE_UNSUPPORTED_COMMAND;
    }

    int ProkyonCamera::GetMultiROI(unsigned *xs, unsigned *ys, unsigned *widths, unsigned *heights, unsigned *length) {
        // TODO
        return DEVICE_UNSUPPORTED_COMMAND;
    }

    int ProkyonCamera::StartSequenceAcquisition(long numImages, double interval_ms, bool stopOnOverflow) {
        // TODO
        return DEVICE_OK;
    }

    int ProkyonCamera::StartSequenceAcquisition(double interval_ms) {
        // TODO
        return DEVICE_OK;
    }

    int ProkyonCamera::StopSequenceAcquisition() {
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
        ss << M_S_CAMERA_DESCRIPTION.c_str() << std::endl;
        if (IS_OK(result)) {
            ss << " " << version;
        }
        return ss.str().c_str();
    }

    // private

    int ProkyonCamera::create_handle() {
        // Initialize SDK
        auto result = DijSDK_Init(&M_S_KEY, 1);
        if (!IS_OK(result)) {
            std::stringstream ss;
            ss << "Error initializing " << M_S_CAMERA_NAME << std::endl;
            log_error(__func__, __LINE__, ss.str());
            return DEVICE_ERR;
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
            std::stringstream ss;
            ss << "Unable to create handle to camera." << std::endl;
            log_error(__func__, __LINE__, ss.str());
            return DEVICE_ERR;
        }

        // Open first camera
        result = DijSDK_OpenCamera(cam_guid[0], &m_camera_handle);
        if (!IS_OK(result)) {
            std::stringstream ss;
            ss << "Camera not valid." << std::endl;
            log_error(__func__, __LINE__, ss.str());
            return DEVICE_ERR;
        }

        assert(IS_OK(result));
        assert(m_camera_handle != nullptr);
        return DEVICE_OK;
    }

    int ProkyonCamera::destroy_handle() {
        if (!valid()) {
            std::stringstream ss;
            ss << "Tried to destroy invalid handle to " << M_S_CAMERA_NAME << std::endl;
            log_error(__func__, __LINE__, ss.str());
            return DEVICE_ERR;
        }

        // TODO check all image buffers are freed
        auto result = DijSDK_CloseCamera(cam());
        if (!IS_OK(result)) {
            std::stringstream ss;
            ss << "Failed to close camera " << M_S_CAMERA_NAME << std::endl;
            log_error(__func__, __LINE__, ss.str());
            return DEVICE_ERR;
        }

        result = DijSDK_Exit();
        if (!IS_OK(result)) {
            std::stringstream ss;
            ss << "Error shutting down DijSDK for " << M_S_CAMERA_NAME << std::endl;
            log_error(__func__, __LINE__, ss.str());
            return DEVICE_ERR;
        }

        m_camera_handle = nullptr;
        return DEVICE_OK;
    }

    bool ProkyonCamera::valid() const {
        return cam() != nullptr;
    }

    const DijSDK_Handle ProkyonCamera::cam() const {
        assert(m_camera_handle != nullptr);
        return m_camera_handle;
    }

    DijSDK_Handle ProkyonCamera::cam() {
        assert(m_camera_handle != nullptr);
        return m_camera_handle;
    }

    ImageInterface *const ProkyonCamera::img() const {
        assert(m_p_image != nullptr);
        return m_p_image.get();
    }

    ImageInterface *ProkyonCamera::img() {
        assert(m_p_image != nullptr);
        return m_p_image.get();
    }

    int ProkyonCamera::log_error(const char *func, const int line, const std::string &message) const {
        std::stringstream ss;
        ss << "Error in " << func << " at line " << line;
        if (!message.empty()) {
            ss << message;
        }
        ss << std::endl;
        LogMessage(ss.str().c_str(), true);
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
        if (valid()) {
            out = DijSDK_HasParameter(m_camera_handle, parameter.id);
        }

        std::stringstream ss;
        ss << parameter.name << " (" << parameter.id << "): " << out;
        LogMessage(ss.str(), true);
    }
}