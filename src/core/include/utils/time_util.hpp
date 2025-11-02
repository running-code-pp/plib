/** 
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-11-02 00:56:22
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-11-02 00:57:09
 * @FilePath: \plib\src\core\src\utils\time_util.cpp
 * @Description:  时间工具函数
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
 */
#ifndef PLIB_CORE_UTILS_TIME_UTIL_HPP_
#define PLIB_CORE_UTILS_TIME_UTIL_HPP_

#include <string>
namespace plib::core::utils
{

    /**
     * @brief:获取当前时间的格式化字符串
     */
    std::string getNowTimeFormatStr(const std::string &format = "%Y-%m-%d %H:%M:%S");
    
}

#endif