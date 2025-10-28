/** 
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-10-28 23:42:44
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-10-29 00:56:29
 * @FilePath: \plib\src\core\include\type_traits.hpp
 * @Description: 类型萃取工具
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
 */
#ifndef PLIB_CORE_TYPE_TRAITS_HPP_
#define PLIB_CORE_TYPE_TRAITS_HPP_
#include <type_traits>
#include <typeindex>
#include <string>

// 辅助 trait：检测是否有 key_type
template <typename T, typename = void>
struct has_key_type : std::false_type {};

template <typename T>
struct has_key_type<T, std::void_t<typename T::key_type>> : std::true_type {};

// 判断是否是序列式容器（必须没有 key_type）
template <typename T, typename = void>
struct is_sequence_container : std::false_type
{
};

// 注意： 这里排除了std::string
template <typename T>
struct is_sequence_container<T, std::void_t<
                                    typename T::value_type,
                                    typename T::iterator,
                                    typename T::size_type,
                                    decltype(std::declval<T>().begin()),
                                    decltype(std::declval<T>().end()),
                                    decltype(std::declval<T>().size()),
                                    std::enable_if_t<!has_key_type<T>::value, void>,
                                    std::enable_if_t<!std::is_same_v<T, std::string>, void>
                                    >> : std::true_type
{
};

template <typename T>
constexpr bool is_sequence_container_v = is_sequence_container<T>::value;

// 判断是否是关联式容器（包括 set 和 map 类）
template <typename T, typename = void>
struct is_associative_container : std::false_type
{
};

// 关联容器：有 key_type 和 find() 方法
template <typename T>
struct is_associative_container<T, std::void_t<
                                       typename T::key_type,
                                       typename T::value_type,
                                       decltype(std::declval<T>().find(std::declval<typename T::key_type>()))>> : std::true_type
{
};

template <typename T>
constexpr bool is_associative_container_v = is_associative_container<T>::value;

// 判断是否是 map 类容器（有 mapped_type）
template <typename T, typename = void>
struct is_map_like : std::false_type
{
};

template <typename T>
struct is_map_like<T, std::void_t<
                         typename T::key_type,
                         typename T::mapped_type,
                         typename T::value_type>> : std::true_type
{
};

template <typename T>
constexpr bool is_map_like_v = is_map_like<T>::value;

// 判断是否是 set 类容器（有 key_type 但没有 mapped_type）
template <typename T>
struct is_set_like : std::bool_constant<
    is_associative_container_v<T> && !is_map_like_v<T>
> {};

template <typename T>
constexpr bool is_set_like_v = is_set_like<T>::value;

// 是否是piar类型

template <typename T, typename = void>
struct is_pair : std::false_type
{
};

template <typename T>
struct is_pair<T, std::void_t<
                      typename T::first_type,
                      typename T::second_type>> : std::true_type
{
};

#endif // PLIB_CORE_TYPE_TRAITS_HPP_