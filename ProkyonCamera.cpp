#define NOMINMAX

#include "ProkyonCamera.h"

#include "Image.h"
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
#include <string>
#include <memory>
#include <map>
#include <numeric>
#include <limits>
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
        m_p_image{nullptr},
        m_p_acq_parameters{nullptr},
        m_p_roi{nullptr},
        m_mapped_scalar_properties{}
    {}

    int ProkyonCamera::Initialize() {
        LogMessage("initializing");
        auto status = m_p_camera->initialize(&M_S_KEY, M_S_CAMERA_NAME, M_S_CAMERA_DESCRIPTION, 0);
        int out = DEVICE_ERR;
        switch (status) {
            case Camera::Status::state_changed:
            {
                std::stringstream ss;
                ss << "using camera:\n";
                ss << "  " << m_p_camera.get() << "\n";
                ss << "  " << m_p_camera->get_error() << "\n";
                ss << "  " << m_p_camera->get_guid() << "\n";
                LogMessage(ss.str());

                LogMessage("creating image buffer");
                m_p_image = std::make_unique<Image>(m_p_camera.get());

                LogMessage("creating roi");
                m_p_roi = std::make_unique<RegionOfInterest>(m_p_camera.get());
                ss.str("");
                ss << "x: (" << m_p_roi->x() << ", " << m_p_roi->x_end() << ") w: " << m_p_roi->w() << std::endl;
                ss << "y: (" << m_p_roi->y() << ", " << m_p_roi->y_end() << ") h: " << m_p_roi->h() << std::endl;
                LogMessage(ss.str());

                LogMessage("creating acquisition parameters");
                m_p_acq_parameters = std::make_unique<AcquisitionParameters>(m_p_camera.get());
                LogMessage("initialized!");

                LogMessage("setting properties");

                LogMessage("ParameterIdImageModeIndex | rw | special discrete");
                NumericProperty image_mode_base(*m_p_camera, ParameterIdImageModeIndex);
                NumericProperty virtual_image_mode_base(*m_p_camera, ParameterIdImageModeVirtualIndex);
                StringProperty virtual_image_mode_name(*m_p_camera, ParameterIdImageModeName);
                std::map<std::string, int> image_mode_forward;
                std::vector<int> range(image_mode_base.range_int()[1], 0);
                std::iota(range.begin(), range.end(), 0);
                for (const auto value : range) {
                    virtual_image_mode_base.set(value);
                    auto key = virtual_image_mode_name.get();
                    key.erase(std::remove(key.begin(), key.end(), ','), key.end());
                    key.erase(std::remove(key.begin(), key.end(), '('), key.end());
                    key.erase(std::remove(key.begin(), key.end(), ')'), key.end());
                    image_mode_forward.emplace(key, value);
                }
                MappedScalarIntProperty image_mode(image_mode_base, image_mode_forward, true);
                for (const auto value : image_mode.range()) {
                    LogMessage(value);
                }
                this->CreatePropertyWithHandler(M_S_IMAGE_MODE_NAME.c_str(), image_mode.range()[0].c_str(), MM::PropertyType::String, false, &ProkyonCamera::update_mapped_scalar_property, false);
                this->SetAllowedValues(M_S_IMAGE_MODE_NAME.c_str(), image_mode.range());
                m_mapped_scalar_properties.emplace(M_S_IMAGE_MODE_NAME, image_mode);
                MappedScalarIntProperty virtual_image_mode(virtual_image_mode_base, image_mode_forward, true);
                m_mapped_scalar_properties.emplace(M_S_VIRTUAL_IMAGE_MODE_NAME, virtual_image_mode);

                LogMessage("ParameterIdImageProcessingOutputFormat | rw | discrete");
                NumericProperty output_format_base(*m_p_camera, ParameterIdImageProcessingOutputFormat);
                std::map<std::string, int> output_format_forward{
                    {"RGB 3 x 8 bpp", DijSDK_EImageFormatRGB888},
                    {"Gray 8 bpp", DijSDK_EImageFormatGrey8},
                    {"Gray 16 bpp", DijSDK_EImageFormatGrey16}
                };
                MappedScalarIntProperty output_format(output_format_base, output_format_forward);
                std::string output_format_name{"Color Mode"};
                this->CreatePropertyWithHandler(output_format_name.c_str(), output_format.range()[0].c_str(), MM::PropertyType::String, false, &ProkyonCamera::update_mapped_scalar_property, false);
                this->SetAllowedValues(output_format_name.c_str(), output_format.range());
                m_mapped_scalar_properties.emplace(output_format_name, output_format);

                LogMessage("ParameterIdImageCaptureGain | rw | scalar double");
                NumericProperty capture_gain(*m_p_camera, ParameterIdImageCaptureGain);
                std::string capture_gain_name{"Capture Gain Target"};
                this->CreateFloatProperty(capture_gain_name.c_str(), capture_gain.get_double(0), false, new CPropertyAction(this, &ProkyonCamera::update_scalar_property), false);
                m_scalar_properties.emplace(capture_gain_name, capture_gain);

                LogMessage("ParameterIdImageProcessingGammaCorrection | rw | scalar double");
                NumericProperty processing_gamma(*m_p_camera, ParameterIdImageProcessingGammaCorrection);
                std::string processing_gamma_name{"Hardware Gamma"};
                this->CreateFloatProperty(processing_gamma_name.c_str(), processing_gamma.get_double(0), false, new CPropertyAction(this, &ProkyonCamera::update_scalar_property), false);
                m_scalar_properties.emplace(processing_gamma_name, processing_gamma);

                LogMessage("ParameterIdImageProcessingContrast | rw | scalar double");
                NumericProperty processing_contrast(*m_p_camera, ParameterIdImageProcessingContrast);
                std::string processing_contrast_name{"Hardware Contrast"};
                this->CreateFloatProperty(processing_contrast_name.c_str(), processing_contrast.get_double(0), false, new CPropertyAction(this, &ProkyonCamera::update_scalar_property), false);
                m_scalar_properties.emplace(processing_contrast_name, processing_contrast);

                LogMessage("ParameterIdImageProcessingSharpness | rw | scalar double");
                NumericProperty processing_sharpness(*m_p_camera, ParameterIdImageProcessingSharpness);
                std::string processing_sharpness_name{"Hardware Sharpness"};
                this->CreateFloatProperty(processing_sharpness_name.c_str(), processing_sharpness.get_double(0), false, new CPropertyAction(this, &ProkyonCamera::update_scalar_property), false);
                m_scalar_properties.emplace(processing_sharpness_name, processing_sharpness);

                LogMessage("ParameterIdImageProcessingHighDynamicRange | rw | scalar bool");
                NumericProperty processing_hdr_enabled(*m_p_camera, ParameterIdImageProcessingHighDynamicRange);
                std::string processing_hdr_enabled_name{"Hardware HDR Enabled"};
                this->CreateFloatProperty(processing_hdr_enabled_name.c_str(), processing_hdr_enabled.get_double(0), false, new CPropertyAction(this, &ProkyonCamera::update_scalar_property), false);
                m_scalar_properties.emplace(processing_hdr_enabled_name, processing_hdr_enabled);

                LogMessage("ParameterIdImageProcessingHdrWeight | rw | scalar double");
                NumericProperty processing_hdr_weight(*m_p_camera, ParameterIdImageProcessingHdrWeight);
                std::string processing_hdr_weight_name{"Hardware HDR Weight"};
                this->CreateFloatProperty(processing_hdr_weight_name.c_str(), processing_hdr_weight.get_double(0), false, new CPropertyAction(this, &ProkyonCamera::update_scalar_property), false);
                m_scalar_properties.emplace(processing_hdr_weight_name, processing_hdr_weight);

                LogMessage("ParameterIdImageProcessingHdrColorGamma | rw | scalar double");
                NumericProperty processing_hdr_color_gamma(*m_p_camera, ParameterIdImageProcessingHdrColorGamma);
                std::string processing_hdr_color_gamma_name{"Hardware HDR Gamma"};
                this->CreateFloatProperty(processing_hdr_color_gamma_name.c_str(), processing_hdr_color_gamma.get_double(0), false, new CPropertyAction(this, &ProkyonCamera::update_scalar_property), false);
                m_scalar_properties.emplace(processing_hdr_color_gamma_name, processing_hdr_color_gamma);

                LogMessage("ParameterIdImageProcessingHdrSmoothFieldSize | rw | scalar double");
                NumericProperty processing_hdr_smooth_field_size(*m_p_camera, ParameterIdImageProcessingHdrSmoothFieldSize);
                std::string processing_hdr_smooth_field_size_gamma_name{"Hardware HDR Smooth Field Size"};
                this->CreateFloatProperty(processing_hdr_smooth_field_size_gamma_name.c_str(), processing_hdr_smooth_field_size.get_double(0), false, new CPropertyAction(this, &ProkyonCamera::update_scalar_property), false);
                m_scalar_properties.emplace(processing_hdr_smooth_field_size_gamma_name, processing_hdr_smooth_field_size);

                // ParameterIdImageProcessingColorBalance | 3 vector double
                // ParameterIdImageProcessingColorBalKeepBrightness | scalar bool
                // ParameterIdImageProcessingSaturation | scalar double
                // ParameterIdExposureControlMode | mapped, keep on for now??
                // ParameterIdExposureControlRoi | 4 vector double
                // ParameterIdExposureControlBrightnessPercentage | scalar int
                // ParameterIdExposureControlExposureLimits | 2 vector int (time in us)
                // ParameterIdExposureControlGainLimits | 2 vector double
                //

                // read only
                // ParameterIdSensorSize | 2 vector int
                // ParameterIdSensorPixelSizeUm | 2 vector int
                // ParameterIdSensorImageCounter | scalar int
                // ParameterIdImageModeSubsampling | 2 vector int
                // ParameterIdImageModeAveraging | 2 vector int
                // ParameterIdImageModeSumming | 2 vector int
                // ParameterIdImageModePreferredAcqMode | mapped
                // ParameterIdGlobalSettingsCameraName | string
                // ParameterIdGlobalSettingsCameraSerialNumber | string
                //

                // hidden, hard-coded
                // ParameterIdImageProcessingWhiteBalanceMode | mapped, hard code to DijSDK_WhiteBalanceModeOff
                // ParameterIdImageProcessingContNoiseFilterCtrlMode | mapped, hard code to DijSDK_ContNoiseFilterCtrlModeSdkControlled

                this->UpdateStatus();
                LogMessage("done");

                out = DEVICE_OK;
                break;
            }
            case Camera::Status::no_change_needed:
            {
                LogMessage("already initialized");
                out = DEVICE_OK;
                break;
            }
            case Camera::Status::failure:
            {
                LogMessage(m_p_camera->get_error());
                out = DEVICE_ERR;
                break;
            }
            default:
                assert(false);
        }

        return out;
    }

    int ProkyonCamera::Shutdown() {
        LogMessage("shutting down");
        auto status = m_p_camera->shutdown();
        int out = DEVICE_ERR;
        switch (status) {
            case Camera::Status::state_changed:
            {
                m_p_acq_parameters.reset(nullptr);
                m_p_roi.reset(nullptr);
                m_p_image.reset(nullptr);
                out = DEVICE_OK;
                break;
            }
            case Camera::Status::no_change_needed:
            {
                LogMessage("already shutdown");
                out = DEVICE_OK;
                break;
            }
            case Camera::Status::failure:
            {
                LogMessage(m_p_camera->get_error());
                out = DEVICE_ERR;
                break;
            }
        }
        return out;
    }

    void ProkyonCamera::GetName(char *name) const {
        LogMessage("getting name");
        CDeviceUtils::CopyLimitedString(name, M_S_CAMERA_NAME.c_str());
    }

    int ProkyonCamera::GetProperty(const char *name, char *value) const {
        LogMessage("getting property");
        std::string name_in{name};
        auto ret = DEVICE_ERR;
        // HACK!
        if (name_in == MM::g_Keyword_Binning) {
            LogMessage("copying");
            auto out = CDeviceUtils::ConvertToString(GetBinning());
            CDeviceUtils::CopyLimitedString(value, out);
            LogMessage("done!");
            ret = DEVICE_OK;
        }
        else {
            ret = CCameraBase<ProkyonCamera>::GetProperty(name, value);
        }
        if (ret == DEVICE_ERR) {
            LogMessage("failed");
            LogMessage(name);
            LogMessage(value);
        }
        return ret;
    }

    int ProkyonCamera::SetProperty(const char *name, const char *value) {
        LogMessage("setting property");
        std::string name_in{name};
        auto ret = DEVICE_ERR;
        if (name_in == MM::g_Keyword_Binning) {
            int value_in = GetBinning();
            try {
                value_in = std::stoi(value);
            }
            catch (std::invalid_argument e) {
                // noop
            }
            catch (std::out_of_range e) {
                // noop
            }
            SetBinning(value_in);
            ret = DEVICE_OK;
        }
        else {
            ret = CCameraBase<ProkyonCamera>::SetProperty(name, value);
        }
        if (ret == DEVICE_ERR) {
            LogMessage("failed");
            LogMessage(name);
            LogMessage(value);
        }
        return ret;
    }

    // CameraBase
    int ProkyonCamera::SnapImage() {
        LogMessage("snapping image");

        m_p_image->acquire();

        std::stringstream ss;
        ss << "image handle: " << m_p_image << "\n";
        LogMessage(ss.str()); ss.str("");
        ss << "component_count: " << m_p_image->get_number_of_components() << std::endl;
        LogMessage(ss.str()); ss.str("");
        ss << "bits_per_channel: " << m_p_image->get_bit_depth() << std::endl;
        LogMessage(ss.str()); ss.str("");
        ss << "bytes_per_pixel: " << m_p_image->get_image_bytes_per_pixel() << std::endl;
        LogMessage(ss.str()); ss.str("");
        ss << "size: " << m_p_image->get_image_width() << ", " << m_p_image->get_image_height() << std::endl;
        LogMessage(ss.str()); ss.str("");
        ss << "bytes: " << m_p_image->get_image_buffer_size() << std::endl;
        LogMessage(ss.str()); ss.str("");
        ss << "binning: " << GetBinning() << std::endl;
        LogMessage(ss.str()); ss.str("");
        ss << "exposure_ms: " << GetExposure() << std::endl;
        LogMessage(ss.str()); ss.str("");
        LogMessage("done");

        return DEVICE_OK;
    }

    const unsigned char *ProkyonCamera::GetImageBuffer() {
        LogMessage("getting image buffer");
        if (m_p_image != nullptr) {
            return m_p_image->get_image_buffer();
        }
        else {
            return nullptr;
        }
    }

    unsigned ProkyonCamera::GetNumberOfComponents() const {
        LogMessage("getting number of components");
        assert(m_p_image != nullptr);
        return m_p_image->get_number_of_components();
    }

    int ProkyonCamera::GetComponentName(unsigned component, char *name) {
        LogMessage("getting component name");
        if (component > GetNumberOfComponents()) {
            LogMessage("failed");
            return DEVICE_NONEXISTENT_CHANNEL;
        }
        else if (m_p_image == nullptr) {
            LogMessage("failed err");
            return DEVICE_ERR;
        }
        else {
            auto out = m_p_image->get_component_name(component);
            CDeviceUtils::CopyLimitedString(name, out.c_str());
            return DEVICE_OK;
        }
    }

    long ProkyonCamera::GetImageBufferSize() const {
        LogMessage("getting image buffer size");
        if (m_p_image == nullptr) {
            LogMessage("failed");
            return DEVICE_NOT_CONNECTED;
        }
        return m_p_image->get_image_buffer_size();
    }

    unsigned ProkyonCamera::GetImageWidth() const {
        LogMessage("getting image buffer width");
        if (m_p_image == nullptr) {
            LogMessage("failed");
            return DEVICE_NOT_CONNECTED;
        }
        else {
            return m_p_image->get_image_width();
        }
    }

    unsigned ProkyonCamera::GetImageHeight() const {
        LogMessage("getting image buffer height");
        if (m_p_image == nullptr) {
            LogMessage("failed");
            return DEVICE_NOT_CONNECTED;
        }
        else {
            return m_p_image->get_image_height();
        }
    }

    unsigned ProkyonCamera::GetImageBytesPerPixel() const {
        LogMessage("getting image bytes per pixel");
        if (m_p_image == nullptr) {
            LogMessage("failed");
            return DEVICE_NOT_CONNECTED;
        }
        else {
            return m_p_image->get_image_bytes_per_pixel();
        }
    }

    unsigned ProkyonCamera::GetBitDepth() const {
        LogMessage("getting bit depth");
        if (m_p_image == nullptr) {
            LogMessage("failed");
            return DEVICE_NOT_CONNECTED;
        }
        else {
            auto v = m_p_image->get_bit_depth();
            std::stringstream ss;
            ss << "bit depth: " << v;
            LogMessage(ss.str());
            return v;
        }
    }

    double ProkyonCamera::GetPixelSizeUm() const {
        return 1.0;
    }

    int ProkyonCamera::GetBinning() const {
        LogMessage("getting binning");
        if (m_p_acq_parameters == nullptr) {
            LogMessage("failed");
            return 1;
        }
        else {
            return m_p_acq_parameters->get_binning();
        }
    }

    int ProkyonCamera::SetBinning(int) {
        LogMessage("binning may not be set directly, choose via imaging mode");
        return DEVICE_OK;
    }

    void ProkyonCamera::SetExposure(double exp_ms) {
        LogMessage("setting exposure");
        if (m_p_acq_parameters == nullptr) {
            LogMessage("failed");
            // noop
        }
        else {
            m_p_acq_parameters->set_exposure_ms(exp_ms);
        }
    }

    double ProkyonCamera::GetExposure() const {
        LogMessage("getting exposure");
        if (m_p_acq_parameters == nullptr) {
            LogMessage("failed");
            return 0.0;
        }
        else {
            return m_p_acq_parameters->get_exposure_ms();
        }
    }

    int ProkyonCamera::SetROI(unsigned x, unsigned y, unsigned xSize, unsigned ySize) {
        LogMessage("setting roi");
        if (m_p_roi == nullptr) {
            LogMessage("failed");
            return DEVICE_NOT_CONNECTED;
        }
        else {
            std::stringstream ss;
            ss << "(" << x << ", " << y << ", " << xSize << ", " << ySize << ")" << std::endl;
            LogMessage(ss.str());
            ROI roi;
            roi.at(X_ind) = x;
            roi.at(Y_ind) = y;
            roi.at(W_ind) = xSize;
            roi.at(H_ind) = ySize;
            m_p_roi->set(roi);
            return DEVICE_OK;
        }
    }

    int ProkyonCamera::GetROI(unsigned &x, unsigned &y, unsigned &xSize, unsigned &ySize) {
        LogMessage("getting roi");
        if (m_p_roi == nullptr) {
            LogMessage("failed");
            return DEVICE_NOT_CONNECTED;
        }
        else {
            x = m_p_roi->x();
            y = m_p_roi->y();
            xSize = m_p_roi->w();
            ySize = m_p_roi->h();
            std::stringstream ss;
            ss << "(" << x << ", " << y << ", " << xSize << ", " << ySize << ")" << std::endl;
            LogMessage(ss.str());
            return DEVICE_OK;
        }
    }

    int ProkyonCamera::ClearROI() {
        LogMessage("clearing roi");
        if (m_p_roi == nullptr) {
            LogMessage("failed");
            return DEVICE_NOT_CONNECTED;
        }
        else {
            m_p_roi->clear();
            return DEVICE_OK;
        }
    }

    int ProkyonCamera::IsExposureSequenceable(bool &isSequencable) const {
        // TODO what is this?
        isSequencable = false;
        return DEVICE_OK;
    }

    int ProkyonCamera::update_scalar_property(MM::PropertyBase *p_prop, MM::ActionType type) {
        auto p = get_property(p_prop);
        switch (p_prop->GetType()) {
            case (MM::PropertyType::Integer):
            {
                auto v = get_integer_scalar_value(p_prop);
                p.normalize_int(v);
                set_integer_scalar_value(v, p_prop);
                p.set(v);
                break;
            }
            case (MM::PropertyType::Float):
            {
                auto v = get_double_scalar_value(p_prop);
                p.normalize_double(v);
                set_double_scalar_value(v, p_prop);
                p.set(v);
                break;
            }
            default:
                assert(false);
        }

        return DEVICE_OK;
    }

    int ProkyonCamera::update_vector_property(MM::PropertyBase *p_prop, MM::ActionType type) {
        auto p = get_property(p_prop);
        switch (p_prop->GetType()) {
            case(MM::PropertyType::Integer):
            {
                bool success = false;
                auto v = get_integer_vector_value(p_prop, success);
                if (!success || v.size() != p.dimension()) {
                    set_integer_vector_value(v, p_prop);
                    return DEVICE_OK;
                }
                p.normalize_int(v);
                set_integer_vector_value(v, p_prop);
                p.set(v);
                break;
            }
            case(MM::PropertyType::Float):
            {
                bool success = false;
                auto v = get_double_vector_value(p_prop, success);
                if (!success || v.size() != p.dimension()) {
                    set_double_vector_value(v, p_prop);
                    return DEVICE_OK;
                }
                p.normalize_double(v);
                set_double_vector_value(v, p_prop);
                p.set(v);
                break;
            }
            default:
                assert(false);
        }

        return DEVICE_OK;
    }

    int ProkyonCamera::get_integer_scalar_value(MM::PropertyBase *p_prop) const {
        long v;
        p_prop->Get(v);
        std::stringstream ss;
        ss << v;
        LogMessage(ss.str());

        // convert long to int
        constexpr int max = std::numeric_limits<int>::max();
        int v_int = 0;
        if (static_cast<long>(max) < v) {
            v_int = max;
        }
        else {
            v_int = v;
        }
        p_prop->Set(static_cast<long>(v_int));

        return v_int;
    }

    void ProkyonCamera::set_integer_scalar_value(int v, MM::PropertyBase *p_prop) {
        p_prop->Set(static_cast<long>(v));
    }

    std::vector<int> ProkyonCamera::get_integer_vector_value(MM::PropertyBase *p_prop, bool &success) const {
        std::string v;
        p_prop->Get(v);
        std::stringstream ss;
        ss << v;
        LogMessage(ss.str());

        // extract values
        std::vector<int> values;
        std::stringstream words(v);
        constexpr char token = ',';
        std::string word;
        int index = 0;
        while (std::getline(words, word, token)) {
            int current;
            try {
                current = std::stoi(word);
            }
            catch (std::invalid_argument) {
                success = false;
            }
            values.push_back(current);
        }
        success = true;
    }

    void ProkyonCamera::set_integer_vector_value(std::vector<int> values, MM::PropertyBase *p_prop) {
        // to string
        std::stringstream ss;
        for (auto i = 0; i < values.size(); ++i) {
            if (0 < i) {
                ss << ", ";
            }
            ss << values[i];
        }
        p_prop->Set(ss.str().c_str());
    }

    double ProkyonCamera::get_double_scalar_value(MM::PropertyBase *p_prop) const {
        double v;
        p_prop->Get(v);
        std::stringstream ss;
        ss << v;
        LogMessage(ss.str());

        return v;
    }

    void ProkyonCamera::set_double_scalar_value(double v, MM::PropertyBase *p_prop) {
        p_prop->Set(v);
    }

    int ProkyonCamera::update_mapped_scalar_property(MM::PropertyBase *p_prop, MM::ActionType type) {
        auto name = p_prop->GetName();
        std::stringstream ss;
        ss << "updating property " << name;
        LogMessage(ss.str());

        if (name == M_S_IMAGE_MODE_NAME) {
            update_image_mode(p_prop, type);
        }
        else {
            std::string v;
            p_prop->Get(v);
            LogMessage(v);

            assert(m_mapped_scalar_properties.count(name));
            auto p = m_mapped_scalar_properties.at(name);
            p.set(v);
        }

        LogMessage("done");
        return DEVICE_OK;
    }

    int ProkyonCamera::update_image_mode(MM::PropertyBase *p_prop, MM::ActionType) {
        std::string s;
        p_prop->Get(s);
        LogMessage(s);

        assert(m_mapped_scalar_properties.count(M_S_IMAGE_MODE_NAME));
        assert(m_mapped_scalar_properties.count(M_S_VIRTUAL_IMAGE_MODE_NAME));

        auto p = m_mapped_scalar_properties.at(M_S_IMAGE_MODE_NAME);
        p.set(s);
        p = m_mapped_scalar_properties.at(M_S_VIRTUAL_IMAGE_MODE_NAME);
        p.set(s);

        LogMessage("done");
        return DEVICE_OK;
    }

    NumericProperty &ProkyonCamera::get_property(MM::PropertyBase *p_prop) {
        auto name = get_mm_property_name(p_prop);
        assert(m_properties.count(name));
        return m_properties.at(name);
    }

    const NumericProperty &ProkyonCamera::get_property(MM::PropertyBase *p_prop) const {
        auto name = get_mm_property_name(p_prop);
        assert(m_properties.count(name));
        return m_properties.at(name);
    }

    std::string ProkyonCamera::get_mm_property_name(MM::PropertyBase *p_prop) const {
        auto name = p_prop->GetName();
        std::stringstream ss;
        ss << "updating property " << name;
        LogMessage(ss.str());
        return name;
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

    const DijSDK_CameraKey ProkyonCamera::M_S_KEY{"C941DD58617B5CA774BF12B70452BF23"};
    const std::string ProkyonCamera::M_S_CAMERA_NAME{"Prokyon"};
    const std::string ProkyonCamera::M_S_CAMERA_DESCRIPTION{"Jenoptik Prokyon"};
    const std::string ProkyonCamera::M_S_IMAGE_MODE_NAME{"Image Mode"};
    const std::string ProkyonCamera::M_S_VIRTUAL_IMAGE_MODE_NAME{"Virtual Image Mode"};

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