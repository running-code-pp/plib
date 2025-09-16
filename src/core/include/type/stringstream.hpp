#ifndef PPCORE_LOGSTREAM_HPP
#define PPCORE_LOGSTREAM_HPP
#include <sstream>
#include <thread>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <type_traits>

namespace plib::core::type{
        class StringStream {
        public:
            StringStream() = default;
            inline StringStream& operator<<(char t) { _ss << t; return *this; }
            inline StringStream& operator<<(bool t) { _ss << (t ? "true" : "false");return *this;}
            inline StringStream& operator<<(const void* t) { _ss << t; return *this; }
            inline StringStream& operator<<(const char* t) { _ss << t; return *this; }
            inline StringStream& operator<<(const std::string& t) { _ss << t; return *this; }
            inline StringStream& operator<<(const std::string_view& t) { _ss << t; return *this; }
            inline StringStream& operator<<(const std::thread::id& t) { _ss << t; return *this; }

            template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, bool>>>
            StringStream& operator<<(T t) {
                _ss << t;
                return *this;
            }

            template<typename T>
            inline StringStream& operator<<(const std::vector<T>& t)
            {
                _ss << '[';
                for (size_t i = 0; i < t.size(); ++i) {
                    *this << t.at(i);
                    if (i < t.size() - 1) {
                        _ss << ',';
                    }
                }
                _ss << ']';
                return *this;
            }


            template<typename Container,
                typename = std::enable_if_t<!std::is_same_v<StringStream&, decltype(std::declval<Container>().begin())>>,
                typename = std::void_t<decltype(std::declval<Container>().begin(), std::declval<Container>().end())>>
                inline StringStream& operator<<(const Container& container) {
                _ss << '[';
                bool first = true;
                for (const auto& elem : container) {
                    if (!first) {
                        _ss << ',';
                    }
                    *this << elem;
                    first = false;
                }
                _ss << ']';
                return *this;
            }

            template<typename T1, typename T2>
            inline StringStream& operator<<(const std::pair<T1, T2>& p) {
                _ss << '(';
                *this << p.first;
                _ss << ", ";
                *this << p.second;
                _ss << ')';
                return *this;
            }

            inline std::string str() const { return _ss.str(); }

        private:
            std::stringstream _ss;
        };
    } // namespace plib::core::type
#endif