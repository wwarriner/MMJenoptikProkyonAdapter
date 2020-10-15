#include "CommonDef.h"

#include "MMDevice/DeviceBase.h"

namespace Prokyon {
    std::string format_error(const char *func, const int line, const std::string &message) {
        std::stringstream ss;
        ss << "Error in " << func << " at line " << line;
        if (!message.empty()) {
            ss << message;
        }
        ss << std::endl;
        return ss.str();
    }
}