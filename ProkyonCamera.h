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
        static const char *get_name(); // done
        static const char *get_description(); // done

    private:
        void setup_numeric_property(DijSDK_EParamId id, std::string id_name, std::string display_name);
        void setup_bool_property(DijSDK_EParamId id, std::string id_name, std::string display_name);
        void setup_string_property(DijSDK_EParamId id, std::string id_name, std::string display_name);
        void setup_image_mode_property();

        int update_numeric_property(MM::PropertyBase *p_prop, MM::ActionType type);
        int update_bool_property(MM::PropertyBase *p_prop, MM::ActionType type);
        int update_string_property(MM::PropertyBase *p_prop, MM::ActionType type);
        int update_discrete_set_property(MM::PropertyBase *p_prop, MM::ActionType type);
        // special case for image mode index and virtual image mode index
        int update_image_mode_property(MM::PropertyBase *p_prop, MM::ActionType type);

        NumericProperty *get_numeric_property(MM::PropertyBase *p_prop);
        const NumericProperty *get_numeric_property(MM::PropertyBase *p_prop) const;
        StringProperty *get_string_property(MM::PropertyBase *p_prop);
        const StringProperty *get_string_property(MM::PropertyBase *p_prop) const;
        std::string get_mm_property_name(MM::PropertyBase *p_prop) const;

        template<typename T>
        std::vector<T> get_numeric_value(MM::PropertyBase *p_prop, bool &success) const;
        template<typename T>
        void set_numeric_value(std::vector<T> values, MM::PropertyBase *p_prop);
        template<typename T>
        static T s_to_num(const std::string &s);

        void log_property_name(const std::string &name) const;
        int log_error(const char *func, const int line, const std::string &message = "") const;

        std::unique_ptr<Camera> m_p_camera;
        std::unique_ptr<Image> m_p_image;
        std::unique_ptr<AcquisitionParameters> m_p_acq_parameters;
        std::unique_ptr<RegionOfInterest> m_p_roi;

        std::map<std::string, std::unique_ptr<StringProperty>> m_string_properties;
        std::map<std::string, std::unique_ptr<NumericProperty>> m_numeric_properties;
        std::map<std::string, std::unique_ptr<BoolProperty>> m_bool_properties;
        std::map<std::string, std::unique_ptr<DiscreteSetProperty>> m_discrete_set_properties;

        static const DijSDK_CameraKey M_S_KEY;
        static const std::string M_S_CAMERA_NAME;
        static const std::string M_S_CAMERA_DESCRIPTION;
        static const std::string M_S_IMAGE_MODE_NAME;
        static const std::string M_S_VIRTUAL_IMAGE_MODE_NAME;
        static const std::string M_S_IMAGE_PROCESSING_OUTPUT_FORMAT_NAME;
        static const std::vector<unsigned char> M_S_TEST_IMAGE;
    };
} // namespace Prokyon

// template function definitions
namespace Prokyon {
    template<typename T>
    std::vector<T> ProkyonCamera::get_numeric_value(MM::PropertyBase *p_prop, bool &success) const {
        std::string v;
        p_prop->Get(v);

        // extract values
        std::vector<T> values;
        std::stringstream words(v);
        const char token = get_numeric_property(p_prop)->delimiter();
        std::string word;
        while (std::getline(words, word, token)) {
            word.erase(std::remove_if(word.begin(), word.end(), [](const char c) {return c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\v' || c == '\f'; }), word.end());
            T current;
            try {
                current = s_to_num<T>(word);
            }
            catch (std::invalid_argument) {
                success = false;
            }
            values.push_back(current);
        }
        success = true;

        return values;
    }

    template<typename T>
    void ProkyonCamera::set_numeric_value(std::vector<T> values, MM::PropertyBase *p_prop) {
        std::stringstream ss;
        for (auto i = 0; i < values.size(); ++i) {
            if (0 < i) {
                ss << get_numeric_property(p_prop)->readable_delimiter();
            }
            ss << values[i];
        }
        p_prop->Set(ss.str().c_str());
    }
}

#endif