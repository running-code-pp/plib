#ifndef PLIB_CORE_UTILS_PATH_UTIL_HPP
#define PLIB_CORE_UTILS_PATH_UTIL_HPP
#include<string>
namespace plib::core::utils{
   std::string get_executable_dir();
   std::string get_executable_path();
   std::string get_executable_name();
   std::string get_current_dir();
}

#endif // PLIB_CORE_UTILS_PATH_UTIL_HPP