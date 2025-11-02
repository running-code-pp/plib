/**
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-11-02 15:42:49
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-11-02 16:15:42
 * @FilePath: \plib\src\core\include\type\bignum.hpp
 * @Description: 基于opensslBIGNUM封装的大数类
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
 */
#ifndef PLIB_CORE_TYPE_BIGNUM_HPP_
#define PLIB_CORE_TYPE_BIGNUM_HPP_

extern "C"
{
#include <openssl/bn.h>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/modes.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
} // extern "C"

#include <vector>
#include <cstdint>
#include <span>
#include <cassert>
#include <type_traits>
#include <cstring>

#ifdef small
#undef small
#endif // small

namespace plib::core::type
{
    // 字节视图工具命名空间 - 使用 C++20 标准库实现
    namespace bytes
    {
        // 类型别名
        using span = std::span<std::byte>;
        using const_span = std::span<const std::byte>;

        // 重载 1: 从可变容器创建可写字节 span
        template <
            typename Container,
            typename = std::enable_if_t<
                !std::is_const_v<Container> &&
                !std::is_same_v<std::remove_cv_t<Container>, span> &&
                !std::is_same_v<std::remove_cv_t<Container>, const_span>>>
        inline span make_span(Container& container)
        {
            auto typed_span = std::span(container);
            return std::as_writable_bytes(typed_span);
        }

        // 重载 2: span 特化 - 直接返回
        inline span make_span(span& container)
        {
            return container;
        }

        // 重载 3: 从 const 容器创建只读字节 span
        template <typename Container>
        inline const_span make_span(const Container& container)
        {
            auto typed_span = std::span(container);
            return std::as_bytes(typed_span);
        }

        // 重载 4: const_span 直接返回
        inline const_span make_span(const_span& container)
        {
            return container;
        }

        // 重载 5: 从 std::span<Type> 创建可写字节 span
        template <typename Type, std::size_t Extent>
        inline span make_span(std::span<Type, Extent> container)
        {
            return std::as_writable_bytes(container);
        }

        // 重载 6: 从 std::span<const Type> 创建只读字节 span
        template <typename Type, std::size_t Extent>
        inline const_span make_span(std::span<const Type, Extent> container)
        {
            return std::as_bytes(container);
        }

        // 重载 7: 从指针+长度创建可写字节 span
        template <typename Type>
        inline span make_span(Type* value, std::size_t count)
        {
            auto typed_span = std::span(value, count);
            return std::as_writable_bytes(typed_span);
        }

        // 重载 8: 从 const 指针+长度创建只读字节 span
        template <typename Type>
        inline const_span make_span(const Type* value, std::size_t count)
        {
            auto typed_span = std::span(value, count);
            return std::as_bytes(typed_span);
        }

        // 额外的实用函数：从字节 span 转换回类型化 span
        template <typename Type>
        inline std::span<Type> as_typed_span(span bytes)
        {
            assert(bytes.size() % sizeof(Type) == 0);
            return std::span<Type>(
                reinterpret_cast<Type*>(bytes.data()),
                bytes.size() / sizeof(Type)
            );
        }

        template <typename Type>
        inline std::span<const Type> as_typed_span(const_span bytes)
        {
            assert(bytes.size() % sizeof(Type) == 0);
            return std::span<const Type>(
                reinterpret_cast<const Type*>(bytes.data()),
                bytes.size() / sizeof(Type)
            );
        }

    } // namespace bytes

    class Context
    {
    public:
        Context() : _data(BN_CTX_new())
        {
        }
        Context(const Context &other) = delete;
        Context(Context &&other) : _data(std::move(other._data))
        {
        }
        Context &operator=(const Context &other) = delete;
        Context &operator=(Context &&other)
        {
            _data = std::move(other._data);
            return *this;
        }
        ~Context()
        {
            if (_data)
            {
                BN_CTX_free(_data);
            }
        }

        BN_CTX *raw() const
        {
            return _data;
        }

    private:
        BN_CTX *_data = nullptr;
    };

    class BigNum
    {
    public:
        BigNum() = default;
        BigNum(const BigNum &other)
            : _data((other.failed() || other.isZero())
                        ? nullptr
                        : BN_dup(other.raw())),
              _failed(other._failed)
        {
        }
        BigNum(BigNum &&other)
            : _data(std::exchange(other._data, nullptr)), _failed(std::exchange(other._failed, false))
        {
        }
        BigNum &operator=(const BigNum &other)
        {
            if (other.failed())
            {
                _failed = true;
            }
            else if (other.isZero())
            {
                clear();
                _failed = false;
            }
            else if (!_data)
            {
                _data = BN_dup(other.raw());
                _failed = false;
            }
            else
            {
                _failed = !BN_copy(raw(), other.raw());
            }
            return *this;
        }
        BigNum &operator=(BigNum &&other)
        {
            std::swap(_data, other._data);
            std::swap(_failed, other._failed);
            return *this;
        }
        ~BigNum()
        {
            clear();
        }

        explicit BigNum(unsigned int word) : BigNum()
        {
            setWord(word);
        }
        explicit BigNum(std::span<const uint8_t> bytes) : BigNum()
        {
            setBytes(bytes);
        }

        BigNum &setWord(unsigned int word)
        {
            if (!word)
            {
                clear();
                _failed = false;
            }
            else
            {
                _failed = !BN_set_word(raw(), word);
            }
            return *this;
        }
        BigNum &setBytes(std::span<const uint8_t> bytes)
        {
            if (bytes.empty())
            {
                clear();
                _failed = false;
            }
            else
            {
                _failed = !BN_bin2bn(
                    reinterpret_cast<const unsigned char *>(bytes.data()),
                    static_cast<int>(bytes.size()),
                    raw());
            }
            return *this;
        }

        BigNum &setAdd(const BigNum &a, const BigNum &b)
        {
            if (a.failed() || b.failed())
            {
                _failed = true;
            }
            else
            {
                _failed = !BN_add(raw(), a.raw(), b.raw());
            }
            return *this;
        }
        BigNum &setSub(const BigNum &a, const BigNum &b)
        {
            if (a.failed() || b.failed())
            {
                _failed = true;
            }
            else
            {
                _failed = !BN_sub(raw(), a.raw(), b.raw());
            }
            return *this;
        }
        BigNum &setMul(
            const BigNum &a,
            const BigNum &b,
            const Context &context = Context())
        {
            if (a.failed() || b.failed())
            {
                _failed = true;
            }
            else
            {
                _failed = !BN_mul(raw(), a.raw(), b.raw(), context.raw());
            }
            return *this;
        }
        BigNum &setModAdd(
            const BigNum &a,
            const BigNum &b,
            const BigNum &m,
            const Context &context = Context())
        {
            if (a.failed() || b.failed() || m.failed())
            {
                _failed = true;
            }
            else if (a.isNegative() || b.isNegative() || m.isNegative())
            {
                _failed = true;
            }
            else if (!BN_mod_add(raw(), a.raw(), b.raw(), m.raw(), context.raw()))
            {
                _failed = true;
            }
            else if (isNegative())
            {
                _failed = true;
            }
            else
            {
                _failed = false;
            }
            return *this;
        }
        BigNum &setModSub(
            const BigNum &a,
            const BigNum &b,
            const BigNum &m,
            const Context &context = Context())
        {
            if (a.failed() || b.failed() || m.failed())
            {
                _failed = true;
            }
            else if (a.isNegative() || b.isNegative() || m.isNegative())
            {
                _failed = true;
            }
            else if (!BN_mod_sub(raw(), a.raw(), b.raw(), m.raw(), context.raw()))
            {
                _failed = true;
            }
            else if (isNegative())
            {
                _failed = true;
            }
            else
            {
                _failed = false;
            }
            return *this;
        }
        BigNum &setModMul(
            const BigNum &a,
            const BigNum &b,
            const BigNum &m,
            const Context &context = Context())
        {
            if (a.failed() || b.failed() || m.failed())
            {
                _failed = true;
            }
            else if (a.isNegative() || b.isNegative() || m.isNegative())
            {
                _failed = true;
            }
            else if (!BN_mod_mul(raw(), a.raw(), b.raw(), m.raw(), context.raw()))
            {
                _failed = true;
            }
            else if (isNegative())
            {
                _failed = true;
            }
            else
            {
                _failed = false;
            }
            return *this;
        }
        BigNum &setModInverse(
            const BigNum &a,
            const BigNum &m,
            const Context &context = Context())
        {
            if (a.failed() || m.failed())
            {
                _failed = true;
            }
            else if (a.isNegative() || m.isNegative())
            {
                _failed = true;
            }
            else if (!BN_mod_inverse(raw(), a.raw(), m.raw(), context.raw()))
            {
                _failed = true;
            }
            else if (isNegative())
            {
                _failed = true;
            }
            else
            {
                _failed = false;
            }
            return *this;
        }
        BigNum &setModExp(
            const BigNum &base,
            const BigNum &power,
            const BigNum &m,
            const Context &context = Context())
        {
            if (base.failed() || power.failed() || m.failed())
            {
                _failed = true;
            }
            else if (base.isNegative() || power.isNegative() || m.isNegative())
            {
                _failed = true;
            }
            else if (!BN_mod_exp(raw(), base.raw(), power.raw(), m.raw(), context.raw()))
            {
                _failed = true;
            }
            else if (isNegative())
            {
                _failed = true;
            }
            else
            {
                _failed = false;
            }
            return *this;
        }
        BigNum &setGcd(
            const BigNum &a,
            const BigNum &b,
            const Context &context = Context())
        {
            if (a.failed() || b.failed())
            {
                _failed = true;
            }
            else if (a.isNegative() || b.isNegative())
            {
                _failed = true;
            }
            else if (!BN_gcd(raw(), a.raw(), b.raw(), context.raw()))
            {
                _failed = true;
            }
            else if (isNegative())
            {
                _failed = true;
            }
            else
            {
                _failed = false;
            }
            return *this;
        }

        [[nodiscard]] bool isZero() const
        {
            return !failed() && (!_data || BN_is_zero(raw()));
        }

        [[nodiscard]] bool isOne() const
        {
            return !failed() && _data && BN_is_one(raw());
        }

        [[nodiscard]] bool isNegative() const
        {
            return !failed() && _data && BN_is_negative(raw());
        }

        [[nodiscard]] bool isPrime(const Context &context = Context()) const
        {
            if (failed() || !_data)
            {
                return false;
            }
            constexpr auto kMillerRabinIterationCount = 64;
            const auto result = BN_is_prime_ex(
                raw(),
                kMillerRabinIterationCount,
                context.raw(),
                nullptr);
            if (result == 1)
            {
                return true;
            }
            else if (result != 0)
            {
                _failed = true;
            }
            return false;
        }

        BigNum &subWord(unsigned int word)
        {
            if (failed())
            {
                return *this;
            }
            else if (!BN_sub_word(raw(), word))
            {
                _failed = true;
            }
            return *this;
        }
        BigNum &divWord(BN_ULONG word, BN_ULONG *mod = nullptr)
        {
            assert(word != 0);

            const auto result = failed()
                                    ? (BN_ULONG)-1
                                    : BN_div_word(raw(), word);
            if (result == (BN_ULONG)-1)
            {
                _failed = true;
            }
            if (mod)
            {
                *mod = result;
            }
            return *this;
        }
        [[nodiscard]] BN_ULONG countModWord(BN_ULONG word) const
        {
            assert(word != 0);

            return failed() ? (BN_ULONG)-1 : BN_mod_word(raw(), word);
        }

        [[nodiscard]] int bitsSize() const
        {
            return failed() ? 0 : BN_num_bits(raw());
        }
        [[nodiscard]] int bytesSize() const
        {
            return failed() ? 0 : BN_num_bytes(raw());
        }

        [[nodiscard]] std::vector<uint8_t> getBytes() const
        {
            if (failed())
            {
                return {};
            }
            auto length = BN_num_bytes(raw());
            auto result = std::vector<uint8_t>(length);
            auto resultSize = BN_bn2bin(
                raw(),
                reinterpret_cast<unsigned char *>(result.data()));
            assert(resultSize == length);
            return result;
        }

        [[nodiscard]] BIGNUM *raw()
        {
            if (!_data)
                _data = BN_new();
            return _data;
        }
        [[nodiscard]] const BIGNUM *raw() const
        {
            if (!_data)
                _data = BN_new();
            return _data;
        }
        [[nodiscard]] BIGNUM *takeRaw()
        {
            return _failed
                       ? nullptr
                   : _data
                       ? std::exchange(_data, nullptr)
                       : BN_new();
        }

        [[nodiscard]] bool failed() const
        {
            return _failed;
        }

        [[nodiscard]] static BigNum Add(const BigNum &a, const BigNum &b)
        {
            return BigNum().setAdd(a, b);
        }
        [[nodiscard]] static BigNum Sub(const BigNum &a, const BigNum &b)
        {
            return BigNum().setSub(a, b);
        }
        [[nodiscard]] static BigNum Mul(
            const BigNum &a,
            const BigNum &b,
            const Context &context = Context())
        {
            return BigNum().setMul(a, b, context);
        }
        [[nodiscard]] static BigNum ModAdd(
            const BigNum &a,
            const BigNum &b,
            const BigNum &mod,
            const Context &context = Context())
        {
            return BigNum().setModAdd(a, b, mod, context);
        }
        [[nodiscard]] static BigNum ModSub(
            const BigNum &a,
            const BigNum &b,
            const BigNum &mod,
            const Context &context = Context())
        {
            return BigNum().setModSub(a, b, mod, context);
        }
        [[nodiscard]] static BigNum ModMul(
            const BigNum &a,
            const BigNum &b,
            const BigNum &mod,
            const Context &context = Context())
        {
            return BigNum().setModMul(a, b, mod, context);
        }
        [[nodiscard]] static BigNum ModInverse(
            const BigNum &a,
            const BigNum &mod,
            const Context &context = Context())
        {
            return BigNum().setModInverse(a, mod, context);
        }
        [[nodiscard]] static BigNum ModExp(
            const BigNum &base,
            const BigNum &power,
            const BigNum &mod,
            const Context &context = Context())
        {
            return BigNum().setModExp(base, power, mod, context);
        }
        [[nodiscard]] static int Compare(const BigNum &a, const BigNum &b)
        {
            return a.failed() ? -1 : b.failed() ? 1
                                                : BN_cmp(a.raw(), b.raw());
        }
        static void Div(
            BigNum *dv,
            BigNum *rem,
            const BigNum &a,
            const BigNum &b,
            const Context &context = Context())
        {
            if (!dv && !rem)
            {
                return;
            }
            else if (a.failed() || b.failed() || !BN_div(dv ? dv->raw() : nullptr, rem ? rem->raw() : nullptr, a.raw(), b.raw(), context.raw()))
            {
                if (dv)
                {
                    dv->_failed = true;
                }
                if (rem)
                {
                    rem->_failed = true;
                }
            }
            else
            {
                if (dv)
                {
                    dv->_failed = false;
                }
                if (rem)
                {
                    rem->_failed = false;
                }
            }
        }
        [[nodiscard]] static BigNum Failed()
        {
            auto result = BigNum();
            result._failed = true;
            return result;
        }

    private:
        void clear()
        {
            BN_clear_free(std::exchange(_data, nullptr));
        }

        mutable BIGNUM *_data = nullptr;
        mutable bool _failed = false;
    };

    namespace details
    {

        template <typename Context, typename Method, typename Arg>
        inline void ShaUpdate(Context context, Method method, Arg &&arg)
        {
            const auto span = bytes::make_span(arg);
            method(context, span.data(), span.size());
        }

        template <typename Context, typename Method, typename Arg, typename... Args>
        inline void ShaUpdate(Context context, Method method, Arg &&arg, Args &&...args)
        {
            const auto span = bytes::make_span(arg);
            method(context, span.data(), span.size());
            ShaUpdate(context, method, args...);
        }

        template <std::size_t Size, typename Method>
        inline void Sha(
            std::span<uint8_t, Size> dst,
            Method method,
            std::span<const uint8_t> data)
        {
            assert(dst.size() >= Size);

            method(
                reinterpret_cast<const unsigned char *>(data.data()),
                data.size(),
                reinterpret_cast<unsigned char *>(dst.data()));
        }

        template <std::size_t Size, typename Method>
        [[nodiscard]] inline std::vector<uint8_t> Sha(
            Method method,
            std::span<const uint8_t> data)
        {
            auto bytes = std::vector<uint8_t>(Size);
            Sha<Size>(std::span<uint8_t, Size>{bytes.data(), Size}, method, data);
            return bytes;
        }

        template <
            std::size_t Size,
            typename Context,
            typename Init,
            typename Update,
            typename Finalize,
            typename... Args,
            typename = std::enable_if_t<(sizeof...(Args) > 1)>>
        [[nodiscard]] std::vector<uint8_t> Sha(
            Context context,
            Init init,
            Update update,
            Finalize finalize,
            Args &&...args)
        {
            auto bytes = std::vector<uint8_t>(Size);

            init(&context);
            ShaUpdate(&context, update, args...);
            finalize(reinterpret_cast<unsigned char *>(bytes.data()), &context);

            return bytes;
        }

        template <
            std::size_t Size,
            typename Evp>
        [[nodiscard]] std::vector<uint8_t> Pbkdf2(
            std::span<const uint8_t> password,
            std::span<const uint8_t> salt,
            int iterations,
            Evp evp)
        {
            auto result = std::vector<uint8_t>(Size);
            PKCS5_PBKDF2_HMAC(
                reinterpret_cast<const char *>(password.data()),
                static_cast<int>(password.size()),
                reinterpret_cast<const unsigned char *>(salt.data()),
                static_cast<int>(salt.size()),
                iterations,
                evp,
                static_cast<int>(result.size()),
                reinterpret_cast<unsigned char *>(result.data()));
            return result;
        }

        constexpr auto kSha1Size = std::size_t(SHA_DIGEST_LENGTH);
        constexpr auto kSha256Size = std::size_t(SHA256_DIGEST_LENGTH);
        constexpr auto kSha512Size = std::size_t(SHA512_DIGEST_LENGTH);

    } // namespace details

    constexpr auto kSha1Size = std::size_t(SHA_DIGEST_LENGTH);
    constexpr auto kSha256Size = std::size_t(SHA256_DIGEST_LENGTH);
    constexpr auto kSha512Size = std::size_t(SHA512_DIGEST_LENGTH);

    [[nodiscard]] inline std::vector<uint8_t> Sha1(std::span<const uint8_t> data)
    {
        return details::Sha<kSha1Size>(SHA1, data);
    }

    inline void Sha1To(std::span<uint8_t, kSha1Size> dst, std::span<const uint8_t> data)
    {
        details::Sha<kSha1Size>(dst, SHA1, data);
    }

    template <
        typename... Args,
        typename = std::enable_if_t<(sizeof...(Args) > 1)>>
    [[nodiscard]] inline std::vector<uint8_t> Sha1(Args &&...args)
    {
        return details::Sha<kSha1Size>(
            SHA_CTX(),
            SHA1_Init,
            SHA1_Update,
            SHA1_Final,
            args...);
    }

    [[nodiscard]] inline std::vector<uint8_t> Sha256(std::span<const uint8_t> data)
    {
        return details::Sha<kSha256Size>(SHA256, data);
    }

    inline void Sha256To(std::span<uint8_t, kSha256Size> dst, std::span<const uint8_t> data)
    {
        details::Sha<kSha256Size>(dst, SHA256, data);
    }

    template <
        typename... Args,
        typename = std::enable_if_t<(sizeof...(Args) > 1)>>
    [[nodiscard]] inline std::vector<uint8_t> Sha256(Args &&...args)
    {
        return details::Sha<kSha256Size>(
            SHA256_CTX(),
            SHA256_Init,
            SHA256_Update,
            SHA256_Final,
            args...);
    }

    [[nodiscard]] inline std::vector<uint8_t> Sha512(std::span<const uint8_t> data)
    {
        return details::Sha<kSha512Size>(SHA512, data);
    }

    inline void Sha512To(std::span<uint8_t, kSha512Size> dst, std::span<const uint8_t> data)
    {
        details::Sha<kSha512Size>(dst, SHA512, data);
    }

    template <
        typename... Args,
        typename = std::enable_if_t<(sizeof...(Args) > 1)>>
    [[nodiscard]] inline std::vector<uint8_t> Sha512(Args &&...args)
    {
        return details::Sha<kSha512Size>(
            SHA512_CTX(),
            SHA512_Init,
            SHA512_Update,
            SHA512_Final,
            args...);
    }

    inline std::vector<uint8_t> Pbkdf2Sha512(
        std::span<const uint8_t> password,
        std::span<const uint8_t> salt,
        int iterations)
    {
        return details::Pbkdf2<kSha512Size>(
            password,
            salt,
            iterations,
            EVP_sha512());
    }

    inline std::vector<uint8_t> HmacSha256(
        std::span<const uint8_t> key,
        std::span<const uint8_t> data)
    {
        auto result = std::vector<uint8_t>(kSha256Size);
        auto length = static_cast<unsigned int>(kSha256Size);

        HMAC(
            EVP_sha256(),
            key.data(),
            static_cast<int>(key.size()),
            reinterpret_cast<const unsigned char *>(data.data()),
            data.size(),
            reinterpret_cast<unsigned char *>(result.data()),
            &length);

        return result;
    }

} // namespace plib::core::type

#endif // PLIB_CORE_TYPE_BIGNUM_HPP_