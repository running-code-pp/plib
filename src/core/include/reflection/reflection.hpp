
#ifndef PLIB_CORE_REFLECTION_REFLECTION_HPP_
#define PLIB_CORE_REFLECTION_REFLECTION_HPP_

#include <string>
#include <type_traits>
#include <sstream>
#include <format>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>
#include <list>
#include <deque>
#include <queue>
#include "type_traits.hpp"

namespace reflection
{
    class TypeDescriptor
    {
    public:
        TypeDescriptor(const char *name, size_t size) : _name(name), _size(size) {}
        virtual ~TypeDescriptor() = default;
        virtual const char *name() const { return _name; }
        virtual size_t size() const { return _size; }
        virtual std::string dump(const void *obj) const = 0;
        virtual nlohmann::json toJson(const void *obj) const = 0;
        virtual void parse(void* obj, const nlohmann::json& j) const = 0;

    protected:
        const char *_name;
        size_t _size;
    };

    template <typename T>
    struct is_basic_supported
        : std::integral_constant<bool,
                                 (std::is_fundamental_v<T> && !std::is_void_v<T>) ||
                                     std::is_same_v<T, std::string>>
    {
    };

    template <typename T, typename std::enable_if<is_basic_supported<T>::value, int>::type = 0>
    class BasicTypeDescriptor : public TypeDescriptor
    {
    public:
        BasicTypeDescriptor() : TypeDescriptor(typeid(T).name(), sizeof(T)) {}

        virtual std::string dump(const void *obj) const override
        {
                return std::format("{}", *(const T *)obj);
        }

        virtual nlohmann::json toJson(const void *obj) const override
        {
            return *(const T *)obj;
        }

        virtual void parse(void* obj, const nlohmann::json& j) const override
        {
            *(T*)obj = j.get<T>();
        }
    };

    // 前向声明序列容器描述符
template <typename ContainerType, 
          std::enable_if_t<is_sequence_container_v<ContainerType>, int> = 0>
    class SequenceContainerDescriptor;

    namespace details
    {
        template <typename T>
        TypeDescriptor *getBasicDescriptor()
        {
            static BasicTypeDescriptor<T> descriptor;
            return &descriptor;
        }

        template <typename ContainerType>
        TypeDescriptor *getSequenceContainerDescriptor()
        {
            static SequenceContainerDescriptor<ContainerType> descriptor;
            return &descriptor;
        }

    } // namespace details

    template <typename T>
    TypeDescriptor *getDescriptor()
    {
        if constexpr (requires { T::getDescription; })
        {
            return &T::getDescription();
        }
        else if constexpr (is_basic_supported<T>::value)
        {
            return details::getBasicDescriptor<T>();
        }
        else if constexpr (is_sequence_container_v<T>)
        {
            return details::getSequenceContainerDescriptor<T>();
        }
        else
        {
            static_assert(is_basic_supported<T>::value,
                          "This type does not support reflection");
            return nullptr;
        }
    }

    struct MemberDescriptor
    {
        const char *name;
        size_t offset;
        TypeDescriptor *type;

        MemberDescriptor(const char *n, size_t o, TypeDescriptor *t)
            : name(n), offset(o), type(t)
        {
        }
    };

    // 序列式容器描述符
 template <typename ContainerType, 
          std::enable_if_t<is_sequence_container_v<ContainerType>, int>>
    class SequenceContainerDescriptor : public TypeDescriptor
    {
    public:
        SequenceContainerDescriptor()
            : TypeDescriptor(typeid(ContainerType).name(), sizeof(ContainerType))
        {
        }
        virtual std::string dump(const void *obj) const override
        {
            std::ostringstream oss;
            oss << "[";
            for (const auto &ele : *reinterpret_cast<const ContainerType *>(obj))
            {
                oss << " " << reflection::getDescriptor<typename ContainerType::value_type>()->dump(&ele)<<",";
            }
            oss << " ]";
            return oss.str();
        }

        virtual nlohmann::json toJson(const void *obj) const override
        {
            nlohmann::json j = nlohmann::json::array();
            for (const auto &ele : *reinterpret_cast<const ContainerType *>(obj))
            {
                j.push_back(reflection::getDescriptor<typename ContainerType::value_type>()->toJson(&ele));
            }
            return j;
        }

        virtual void parse(void* obj, const nlohmann::json& j) const override
        {
            auto* container = reinterpret_cast<ContainerType*>(obj);
            container->clear();
            if constexpr (is_basic_supported<typename ContainerType::value_type>::value)
            {
               *container = j.get<ContainerType>();
            }
            else
            {
                    for (const auto& item : j)
                    {
                         typename ContainerType::value_type element;
                         reflection::getDescriptor<typename ContainerType::value_type>()->parse(&element, item);
                         container->insert(container->end(), std::move(element));
                    }
            }
        }
    };

    // 关联式容器描述符
    class AllocatedContainerDescriptor : public TypeDescriptor
    {
    public:
        AllocatedContainerDescriptor() : TypeDescriptor("AllocatedContainer", 0) {}
        
        virtual std::string dump(const void* obj) const override
        {
            return "{}";
        }

        virtual nlohmann::json toJson(const void* obj) const override
        {
            return nlohmann::json::object();
        }

        virtual void parse(void* obj, const nlohmann::json& j) const override
        {
            // TODO: 实现关联容器的解析
        }
    };

    class StructDescriptor : public TypeDescriptor
    {
        std::vector<MemberDescriptor> members;

    public:
        StructDescriptor(const char *name, size_t size)
            : TypeDescriptor(name, size)
        {
        }

        void addMember(const char *name, size_t offset, TypeDescriptor *type)
        {
            members.emplace_back(name, offset, type);
        }

        const std::vector<MemberDescriptor> &getMembers() const { return members; }

        virtual std::string dump(const void *obj) const override
        {
            return toJson(obj).dump(4);
        }

        virtual nlohmann::json toJson(const void *obj) const override
        {
            nlohmann::json json;
            for (const auto &mem : members)
            {
                json[mem.name] = mem.type->toJson(reinterpret_cast<const char *>(obj) + mem.offset);
            }
            return json;
        }

        virtual void parse(void* obj, const nlohmann::json& j) const override
        {
            for (const auto& mem : members)
            {
                if (j.contains(mem.name))
                {
                    void* member_addr = reinterpret_cast<char*>(obj) + mem.offset;
                    mem.type->parse(member_addr, j[mem.name]);
                }
            }
        }
    };

    template <typename T>
    concept has_get_description = requires {
        T::getDescription();
    };

    template <typename MemType, typename StructType>
        requires has_get_description<StructType>
    std::optional<MemType> get(StructType *data, const std::string &name)
    {
        auto desc = StructType::getDescription();

        for (const auto &meta : desc.getMembers())
        {
            if (std::strcmp(meta.name, name.c_str()) == 0)
            {
                void *member_addr = reinterpret_cast<char *>(data) + meta.offset;
                return std::optional<MemType>(*reinterpret_cast<MemType *>(member_addr));
            }
        }

        return std::nullopt;
    }

    // 从 JSON 解析对象（使用反射系统）
    template<typename T>  requires has_get_description<T>
    T parse_json(const std::string& json_str)
    {
        nlohmann::json j = nlohmann::json::parse(json_str);
        T obj;
        auto& desc = T::getDescription();
        desc.parse(&obj, j);
        return obj;
    }

} // namespace reflection

#define ENABLE_REFLECT(NAME, SIZE)                                           \
    inline static reflection::StructDescriptor &getDescription()             \
    {                                                                        \
        static reflection::StructDescriptor desc{NAME, SIZE};                \
        return desc;                                                         \
    }                                                                        \
    template <typename Class, typename MemberType>                           \
    static void _registerMember(const char *name, MemberType Class::*ptr)    \
    {                                                                        \
        size_t offset = reinterpret_cast<size_t>(                            \
            &(reinterpret_cast<Class const volatile *>(0)->*ptr));           \
        getDescription().addMember(name, offset,                             \
                                   reflection::getDescriptor<MemberType>()); \
    }

#define REFLECTABLE(class_name, member_type, member_name) \
    member_type member_name;                              \
    inline static const bool _reflectionInit_##member_name = [] {            \
        class_name::_registerMember<class_name, member_type>(                \
            #member_name, &class_name::member_name);                         \
        return true; }();

#endif // PLIB_CORE_REFLECTION_REFLECTION_HPP_