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

// TODO
// Need to fix a few errors:
//  (1) can't do multi-d acq because "Color Mode" prop is not valid name?
//  (2) can't convert string to java.awt.color?
//  (3) can't find color property??
//  (4) something generally wrong with color mode property...
//  (5) document crash when trying to change color mode while live (shouldn't happen??)

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
        m_discrete_set_properties{}
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
                LogMessage(m_p_image->to_string());

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

                // special cases
                setup_image_mode_property();

                // read write
                setup_numeric_property(ParameterIdImageCaptureGain, "ParameterIdImageCaptureGain", "Image Capture-Gain Target");
                setup_numeric_property(ParameterIdImageProcessingGammaCorrection, "ParameterIdImageProcessingGammaCorrection", "Image Processing-Gamma");
                setup_numeric_property(ParameterIdImageProcessingContrast, "ParameterIdImageProcessingContrast", "Image Processing-Contrast");
                setup_numeric_property(ParameterIdImageProcessingSharpness, "ParameterIdImageProcessingSharpness", "Image Processing-Sharpness");
                setup_bool_property(ParameterIdImageProcessingHighDynamicRange, "ParameterIdImageProcessingHighDynamicRange", "Image Processing-HDR-Enabled");
                setup_numeric_property(ParameterIdImageProcessingHdrWeight, "ParameterIdImageProcessingHdrWeight", "Image Processing-HDR-Weight");
                setup_numeric_property(ParameterIdImageProcessingHdrColorGamma, "ParameterIdImageProcessingHdrColorGamma", "Image Processing-HDR-Gamma");
                setup_numeric_property(ParameterIdImageProcessingHdrSmoothFieldSize, "ParameterIdImageProcessingHdrSmoothFieldSize", "Image Processing-HDR-Smooth Field Size");
                setup_numeric_property(ParameterIdImageProcessingColorBalance, "ParameterIdImageProcessingColorBalance", "Image Processing-HDR-Color Balance");
                setup_bool_property(ParameterIdImageProcessingColorBalKeepBrightness, "ParameterIdImageProcessingColorBalKeepBrightness", "Image Processing-HDR-Color Balance-Keep Brightness?");
                setup_numeric_property(ParameterIdImageProcessingSaturation, "ParameterIdImageProcessingSaturation", "Image Processing-Saturation");

                // read only
                setup_numeric_property(ParameterIdSensorSize, "ParameterIdSensorSize", "Sensor-Size (px)");
                setup_numeric_property(ParameterIdSensorPixelSizeUm, "ParameterIdSensorPixelSizeUm", "Sensor-Pixel Size (um)");
                setup_numeric_property(ParameterIdImageModeSubsampling, "ParameterIdImageModeSubsampling", "Image Mode-Subsampling Bin Size (px)");
                setup_numeric_property(ParameterIdImageModeAveraging, "ParameterIdImageModeAveraging", "Image Mode-Averaging Bin Size (px)");
                setup_numeric_property(ParameterIdImageModeSumming, "ParameterIdImageModeSumming", "Image Mode-Summing Bin Size (px)");
                setup_string_property(ParameterIdGlobalSettingsCameraName, "ParameterIdGlobalSettingsCameraName", "Global-Camera Name");
                // TODO better error checking
                //setup_string_property(ParameterIdGlobalSettingsCameraSerialNumber, "ParameterIdGlobalSettingsCameraSerialNumber", "Global-Camera Serial Number");

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
        auto success = m_p_image->acquire();
        if (!success) {
            return DEVICE_ERR;
        }
        else {
            LogMessage(m_p_image->to_string());
        }
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
            return m_p_image->get_bit_depth();
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
    void ProkyonCamera::setup_numeric_property(DijSDK_EParamId id, std::string id_name, std::string display_name) {
        LogMessage("setting up: " + id_name);
        auto p_property = std::make_unique<NumericProperty>(*m_p_camera, id);
        std::stringstream ss;
        ss << id_name << " | " << p_property->short_specification_to_string();
        LogMessage(ss.str());
        auto p_callback = new CPropertyAction(this, &ProkyonCamera::update_numeric_property);
        this->CreateStringProperty(display_name.c_str(), p_property->vector_to_string().c_str(), !p_property->writeable(), p_callback, false);
        m_numeric_properties[display_name] = std::move(p_property);
    }

    void ProkyonCamera::setup_bool_property(DijSDK_EParamId id, std::string id_name, std::string display_name) {
        LogMessage("setting up: " + id_name);
        auto p_property = std::make_unique<BoolProperty>(*m_p_camera, id);
        std::stringstream ss;
        ss << id_name << " | " << p_property->short_specification_to_string();
        LogMessage(ss.str());
        this->CreatePropertyWithHandler(display_name.c_str(), p_property->range()[0].c_str(), MM::PropertyType::String, !p_property->writeable(), &ProkyonCamera::update_bool_property, false);
        this->SetAllowedValues(display_name.c_str(), p_property->range());
        m_bool_properties[display_name] = std::move(p_property);
    }

    void ProkyonCamera::setup_string_property(DijSDK_EParamId id, std::string id_name, std::string display_name) {
        LogMessage("setting up: " + id_name);
        auto p_property = std::make_unique<StringProperty>(*m_p_camera, id);
        std::stringstream ss;
        ss << id_name << " | " << "string";
        LogMessage(ss.str());
        auto p_callback = new CPropertyAction(this, &ProkyonCamera::update_string_property);
        this->CreateStringProperty(display_name.c_str(), p_property->get().c_str(), !p_property->writeable(), p_callback, false);
        m_string_properties[display_name] = std::move(p_property);
    }

    void ProkyonCamera::setup_image_mode_property() {
        // TODO
        LogMessage("ParameterIdImageModeIndex | rw | special discrete");
        NumericProperty image_mode_base(*m_p_camera, ParameterIdImageModeIndex);
        NumericProperty virtual_image_mode_base(*m_p_camera, ParameterIdImageModeVirtualIndex);
        StringProperty virtual_image_mode_name(*m_p_camera, ParameterIdImageModeName);
        std::map<std::string, int> image_mode_forward;
        std::vector<int> range(image_mode_base.range_int()[1], 0);
        std::iota(range.begin(), range.end(), 0);
        for (const auto value : range) {
            virtual_image_mode_base.set(std::vector<int>{value});
            auto key = virtual_image_mode_name.get();
            key.erase(std::remove(key.begin(), key.end(), ','), key.end());
            key.erase(std::remove(key.begin(), key.end(), '('), key.end());
            key.erase(std::remove(key.begin(), key.end(), ')'), key.end());
            image_mode_forward.emplace(key, value);
        }
        auto p_image_mode = std::make_unique<DiscreteSetProperty>(*m_p_camera, ParameterIdImageModeIndex, image_mode_forward, true);
        for (const auto value : p_image_mode->range()) {
            LogMessage(value);
        }
        this->CreatePropertyWithHandler(M_S_IMAGE_MODE_NAME.c_str(), p_image_mode->range()[0].c_str(), MM::PropertyType::String, false, &ProkyonCamera::update_discrete_set_property, false);
        this->SetAllowedValues(M_S_IMAGE_MODE_NAME.c_str(), p_image_mode->range());
        m_discrete_set_properties[M_S_IMAGE_MODE_NAME] = std::move(p_image_mode);

        auto p_virtual_image_mode = std::make_unique<DiscreteSetProperty>(*m_p_camera, ParameterIdImageModeVirtualIndex, image_mode_forward, true);
        m_discrete_set_properties[M_S_VIRTUAL_IMAGE_MODE_NAME] = std::move(p_virtual_image_mode);

        LogMessage("ParameterIdImageProcessingOutputFormat | rw | discrete");
        NumericProperty output_format_base(*m_p_camera, ParameterIdImageProcessingOutputFormat);
        std::map<std::string, int> output_format_forward{
            {"RGB 3 x 8 bpp", DijSDK_EImageFormatRGB888},
            {"Gray 8 bpp", DijSDK_EImageFormatGrey8},
            {"Gray 16 bpp", DijSDK_EImageFormatGrey16}
        };
        auto p_output_format = std::make_unique<DiscreteSetProperty>(*m_p_camera, ParameterIdImageProcessingOutputFormat, output_format_forward, false);
        this->CreatePropertyWithHandler(M_S_IMAGE_PROCESSING_OUTPUT_FORMAT_NAME.c_str(), p_output_format->range()[0].c_str(), MM::PropertyType::String, false, &ProkyonCamera::update_discrete_set_property, false);
        this->SetAllowedValues(M_S_IMAGE_PROCESSING_OUTPUT_FORMAT_NAME.c_str(), p_output_format->range());
        m_discrete_set_properties[M_S_IMAGE_PROCESSING_OUTPUT_FORMAT_NAME] = std::move(p_output_format);
    }

    int ProkyonCamera::update_numeric_property(MM::PropertyBase *p_prop, MM::ActionType) {
        auto p = get_numeric_property(p_prop);
        std::stringstream ss;
        ss << "updating property " << p_prop->GetName();
        LogMessage(ss.str());
        switch (p->type()) {
            case(NumericProperty::Type::Int):
            {
                bool success = false;
                auto v = get_integer_value(p_prop, success);
                if (!success || v.size() != p->dimension() || !p->writeable()) {
                    set_integer_value(p->get_int(), p_prop);
                    return DEVICE_OK;
                }
                else {
                    p->normalize_int(v);
                    set_integer_value(v, p_prop);
                    p->set(v);
                }
                break;
            }
            case(NumericProperty::Type::Double):
            {
                bool success = false;
                auto v = get_double_value(p_prop, success);
                if (!success || v.size() != p->dimension() || !p->writeable()) {
                    set_double_value(p->get_double(), p_prop);
                    return DEVICE_OK;
                }
                else {
                    p->normalize_double(v);
                    set_double_value(v, p_prop);
                    p->set(v);
                }
                break;
            }
            default:
                assert(false);
        }

        return DEVICE_OK;
    }

    int ProkyonCamera::update_string_property(MM::PropertyBase *p_prop, MM::ActionType) {
        std::stringstream ss;
        ss << "updating property " << p_prop->GetName();
        LogMessage(ss.str());

        auto p = get_string_property(p_prop);
        p_prop->Set(p->get().c_str());
        return DEVICE_OK;
    }

    int ProkyonCamera::update_bool_property(MM::PropertyBase *p_prop, MM::ActionType) {
        auto name = get_mm_property_name(p_prop);
        log_property_name(name);

        std::string v;
        p_prop->Get(v);
        assert(m_bool_properties.count(name));
        auto p = m_bool_properties.at(name).get();
        p->set(v);
        return DEVICE_OK;
    }

    int ProkyonCamera::update_discrete_set_property(MM::PropertyBase *p_prop, MM::ActionType type) {
        auto name = get_mm_property_name(p_prop);
        log_property_name(name);

        if (name == M_S_IMAGE_MODE_NAME) {
            update_image_mode_property(p_prop, type);
        }
        else {
            std::string v;
            p_prop->Get(v);

            assert(m_discrete_set_properties.count(name));
            auto p = m_discrete_set_properties.at(name).get();
            p->set(v);
        }

        if (name == M_S_IMAGE_PROCESSING_OUTPUT_FORMAT_NAME) {
            if (!m_p_image->update()) {
                return DEVICE_ERR;
            }
        }

        return DEVICE_OK;
    }

    int ProkyonCamera::update_image_mode_property(MM::PropertyBase *p_prop, MM::ActionType) {
        std::string s;
        p_prop->Get(s);

        assert(m_discrete_set_properties.count(M_S_IMAGE_MODE_NAME));
        assert(m_discrete_set_properties.count(M_S_VIRTUAL_IMAGE_MODE_NAME));

        auto p = m_discrete_set_properties.at(M_S_IMAGE_MODE_NAME).get();
        p->set(s);
        p = m_discrete_set_properties.at(M_S_VIRTUAL_IMAGE_MODE_NAME).get();
        p->set(s);

        return DEVICE_OK;
    }

    std::vector<int> ProkyonCamera::get_integer_value(MM::PropertyBase *p_prop, bool &success) const {
        std::string v;
        p_prop->Get(v);

        // extract values
        std::vector<int> values;
        std::stringstream words(v);
        const char token = get_numeric_property(p_prop)->DELIMITER;
        std::string word;
        while (std::getline(words, word, token)) {
            word.erase(std::remove_if(word.begin(), word.end(), [](const char c) {return c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\v' || c == '\f'; }), word.end());
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

        return values;
    }

    void ProkyonCamera::set_integer_value(std::vector<int> values, MM::PropertyBase *p_prop) {
        // to string
        std::stringstream ss;

        for (auto i = 0; i < values.size(); ++i) {
            if (0 < i) {
                ss << " " << get_numeric_property(p_prop)->DELIMITER << " ";
            }
            ss << values[i];
        }
        p_prop->Set(ss.str().c_str());
    }

    std::vector<double> ProkyonCamera::get_double_value(MM::PropertyBase *p_prop, bool &success) const {
        std::string v;
        p_prop->Get(v);

        // extract values
        std::vector<double> values;
        std::stringstream words(v);
        const char token = get_numeric_property(p_prop)->DELIMITER;
        std::string word;
        while (std::getline(words, word, token)) {
            word.erase(std::remove_if(word.begin(), word.end(), [](const char c) {return c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\v' || c == '\f'; }), word.end());
            double current;
            try {
                current = std::stod(word);
            }
            catch (std::invalid_argument) {
                success = false;
            }
            values.push_back(current);
        }
        success = true;

        return values;
    }

    void ProkyonCamera::set_double_value(std::vector<double> values, MM::PropertyBase *p_prop) {
        // to string
        std::stringstream ss;
        ss << std::setprecision(4);
        for (auto i = 0; i < values.size(); ++i) {
            if (0 < i) {
                ss << " " << get_numeric_property(p_prop)->DELIMITER << " ";
            }
            ss << values[i];
        }
        p_prop->Set(ss.str().c_str());
    }

    NumericProperty *ProkyonCamera::get_numeric_property(MM::PropertyBase *p_prop) {
        auto name = get_mm_property_name(p_prop);
        assert(m_numeric_properties.count(name));
        return m_numeric_properties.at(name).get();
    }

    const NumericProperty *ProkyonCamera::get_numeric_property(MM::PropertyBase *p_prop) const {
        auto name = get_mm_property_name(p_prop);
        assert(m_numeric_properties.count(name));
        return m_numeric_properties.at(name).get();
    }

    StringProperty *ProkyonCamera::get_string_property(MM::PropertyBase *p_prop) {
        auto name = get_mm_property_name(p_prop);
        assert(m_string_properties.count(name));
        return m_string_properties.at(name).get();
    }

    const StringProperty *ProkyonCamera::get_string_property(MM::PropertyBase *p_prop) const {
        auto name = get_mm_property_name(p_prop);
        assert(m_string_properties.count(name));
        return m_string_properties.at(name).get();
    }

    std::string ProkyonCamera::get_mm_property_name(MM::PropertyBase *p_prop) const {
        auto name = p_prop->GetName();
        return name;
    }

    void ProkyonCamera::log_property_name(const std::string &name) const {
        std::stringstream ss;
        ss << "updating property " << name;
        LogMessage(ss.str());
    }

    int ProkyonCamera::log_error(const char *func, const int line, const std::string &message) const {
        LogMessage(format_error(func, line, message), true);
        return DEVICE_ERR;
    }

    const DijSDK_CameraKey ProkyonCamera::M_S_KEY{"C941DD58617B5CA774BF12B70452BF23"};
    const std::string ProkyonCamera::M_S_CAMERA_NAME{"Prokyon"};
    const std::string ProkyonCamera::M_S_CAMERA_DESCRIPTION{"Jenoptik Prokyon"};
    const std::string ProkyonCamera::M_S_IMAGE_MODE_NAME{"Image Mode"};
    const std::string ProkyonCamera::M_S_VIRTUAL_IMAGE_MODE_NAME{"Virtual Image Mode"};
    const std::string ProkyonCamera::M_S_IMAGE_PROCESSING_OUTPUT_FORMAT_NAME{"Image Processing-Color Mode"};
} // namespace Prokyon