/**
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-10-21 22:24:28
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-10-21 22:26:24
 * @FilePath: \plib\src\core\include\type\move_only_function.hpp
 * @Description: 只允许移动的函数对象，没有RTTI开销
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
 */
#ifndef PLIB_CORE_TYPE_MOVE_ONLY_FUNCTION_HPP_
#define PLIB_CORE_TYPE_MOVE_ONLY_FUNCTION_HPP_

#include <type_traits>
#include <cstddef>
#include <memory>
#include <utility>
#include <exception>
#include <functional>

namespace plib::core::type
{
    template <typename Signature>
    class move_only_function;
    namespace detail
    {
     // 用于检查返回类型，ResultType和RetType
     // ResultType实际返回类型, RetType为期望返回类型
     template <typename ResultType, typename RetType, typename = void>
     struct RetTypeCheck: std::false_type{};

     // 期望返回值为void的特化
     template <typename ResultType, typename RetType>
     struct RetTypeCheck<ResultType, RetType, std::void_t<typename ResultType::type>>: std::true_type{};

    // 期望返回值非void的特化
    template <typename ResultType, typename RetType>
    struct RetTypeCheck<ResultType, RetType, std::void_t<typename ResultType::type>>: std::true_type{};

    

    }
}

#endif // PLIB_CORE_TYPE_MOVE_ONLY_FUNCTION_HPP_