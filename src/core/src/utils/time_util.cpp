#include "utils/time_util.hpp"
#include <ctime>

namespace plib::core::utils
{

    std::string getNowTimeFormatStr(const std::string &format)
    {
        std::time_t t = std::time(nullptr);
        char buf[100];
        if (std::strftime(buf, sizeof(buf), format.c_str(), std::localtime(&t)))
        {
            return std::string(buf);
        }
        return "";
    }

} // namespace plib::core::utils