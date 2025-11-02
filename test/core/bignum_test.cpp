/**
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-11-02
 * @Description: BigNum 类的完整测试（使用 Google Test）
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
 */

#include <gtest/gtest.h>
#include "type/bignum.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

using namespace plib::core::type;

// 辅助函数：打印字节数组为十六进制
void printHex(const std::string& label, const std::vector<uint8_t>& bytes) {
    std::cout << label << ": ";
    for (auto byte : bytes) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(byte);
    }
    std::cout << std::dec << std::endl;
}

// 辅助函数：打印 BigNum 的值（通过字节表示）
void printBigNum(const std::string& label, const BigNum& bn) {
    if (bn.failed()) {
        std::cout << label << ": <FAILED>" << std::endl;
    } else if (bn.isZero()) {
        std::cout << label << ": 0" << std::endl;
    } else {
        auto bytes = bn.getBytes();
        printHex(label, bytes);
        std::cout << "  (bits: " << bn.bitsSize() << ", bytes: " << bn.bytesSize() << ")" << std::endl;
    }
}

// 测试基本构造和赋值
TEST(BigNumTest, BasicConstruction) {
    // 默认构造
    BigNum bn1;
    EXPECT_TRUE(bn1.isZero());
    EXPECT_FALSE(bn1.failed());
    
    // 从 unsigned int 构造
    BigNum bn2(12345);
    EXPECT_FALSE(bn2.isZero());
    EXPECT_FALSE(bn2.failed());
    
    // 从字节数组构造
    std::vector<uint8_t> bytes = {0x01, 0x02, 0x03, 0x04};
    BigNum bn3(std::span{bytes});
    EXPECT_FALSE(bn3.isZero());
    EXPECT_FALSE(bn3.failed());
    
    // 拷贝构造
    BigNum bn4(bn2);
    EXPECT_EQ(BigNum::Compare(bn2, bn4), 0);
    
    // 移动构造
    BigNum bn5(std::move(bn4));
    EXPECT_EQ(BigNum::Compare(bn2, bn5), 0);
}

// 测试基本运算
TEST(BigNumTest, BasicOperations) {
    BigNum a(1000);
    BigNum b(500);
    
    // 加法
    auto sum = BigNum::Add(a, b);
    EXPECT_FALSE(sum.failed());
    
    // 减法
    auto diff = BigNum::Sub(a, b);
    EXPECT_FALSE(diff.failed());
    
    // 乘法
    auto prod = BigNum::Mul(a, b);
    EXPECT_FALSE(prod.failed());
    
    // 除法
    BigNum quotient, remainder;
    BigNum::Div(&quotient, &remainder, a, b);
    EXPECT_FALSE(quotient.failed());
    EXPECT_FALSE(remainder.failed());
}

// 测试模运算
TEST(BigNumTest, ModularOperations) {
    BigNum a(100);
    BigNum b(50);
    BigNum m(97);  // 质数模数
    
    Context ctx;
    
    // 模加法
    auto modAdd = BigNum::ModAdd(a, b, m, ctx);
    EXPECT_FALSE(modAdd.failed());
    EXPECT_FALSE(modAdd.isNegative());
    
    // 模减法
    auto modSub = BigNum::ModSub(a, b, m, ctx);
    EXPECT_FALSE(modSub.failed());
    EXPECT_FALSE(modSub.isNegative());
    
    // 模乘法
    auto modMul = BigNum::ModMul(a, b, m, ctx);
    EXPECT_FALSE(modMul.failed());
    EXPECT_FALSE(modMul.isNegative());
    
    // 模逆元
    BigNum a_inv = BigNum::ModInverse(a, m, ctx);
    EXPECT_FALSE(a_inv.failed());
    
    // 验证：a * a_inv mod m = 1
    auto verify = BigNum::ModMul(a, a_inv, m, ctx);
    EXPECT_TRUE(verify.isOne());
}

// 测试模幂运算
TEST(BigNumTest, ModularExponentiation) {
    BigNum base(7);
    BigNum power(10);
    BigNum modulus(13);
    
    Context ctx;
    auto result = BigNum::ModExp(base, power, modulus, ctx);
    EXPECT_FALSE(result.failed());
    EXPECT_FALSE(result.isNegative());
}

// 测试 GCD（最大公约数）
TEST(BigNumTest, GCD) {
    BigNum a(48);
    BigNum b(18);
    
    Context ctx;
    BigNum gcd;
    gcd.setGcd(a, b, ctx);
    
    EXPECT_FALSE(gcd.failed());
    
    // gcd(48, 18) 应该是 6
    BigNum expected(6);
    EXPECT_EQ(BigNum::Compare(gcd, expected), 0);
}

// 测试质数检测
TEST(BigNumTest, PrimeChecking) {
    Context ctx;
    
    // 已知的质数
    std::vector<unsigned int> primes = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47};
    for (auto p : primes) {
        BigNum bn(p);
        EXPECT_TRUE(bn.isPrime(ctx)) << "Expected " << p << " to be prime";
    }
    
    // 已知的合数
    std::vector<unsigned int> composites = {4, 6, 8, 9, 10, 12, 14, 15, 16, 18, 20, 21, 22, 24};
    for (auto c : composites) {
        BigNum bn(c);
        EXPECT_FALSE(bn.isPrime(ctx)) << "Expected " << c << " to be composite";
    }
    
    // 大质数
    BigNum largePrime(65537);  // 费马数 F4
    EXPECT_TRUE(largePrime.isPrime(ctx));
}

// 测试字节转换
TEST(BigNumTest, BytesConversion) {
    std::vector<uint8_t> original = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE};
    BigNum bn;
    bn.setBytes(std::span<uint8_t>{original});
    
    auto retrieved = bn.getBytes();
    EXPECT_EQ(original, retrieved);
}

// 测试位和字节大小
TEST(BigNumTest, Sizes) {
    BigNum bn1(255);  // 8 位
    EXPECT_EQ(bn1.bitsSize(), 8);
    EXPECT_EQ(bn1.bytesSize(), 1);
    
    BigNum bn2(256);  // 9 位
    EXPECT_EQ(bn2.bitsSize(), 9);
    EXPECT_EQ(bn2.bytesSize(), 2);
    
    BigNum bn3(65535);  // 16 位
    EXPECT_EQ(bn3.bitsSize(), 16);
    EXPECT_EQ(bn3.bytesSize(), 2);
}

// 测试比较操作
TEST(BigNumTest, Comparison) {
    BigNum a(100);
    BigNum b(200);
    BigNum c(100);
    
    EXPECT_LT(BigNum::Compare(a, b), 0);  // 100 < 200
    EXPECT_GT(BigNum::Compare(b, a), 0);  // 200 > 100
    EXPECT_EQ(BigNum::Compare(a, c), 0);  // 100 == 100
}

// 测试状态检查
TEST(BigNumTest, StateChecks) {
    BigNum zero;
    EXPECT_TRUE(zero.isZero());
    EXPECT_FALSE(zero.isOne());
    EXPECT_FALSE(zero.isNegative());
    
    BigNum one(1);
    EXPECT_FALSE(one.isZero());
    EXPECT_TRUE(one.isOne());
    EXPECT_FALSE(one.isNegative());
}

// 测试 SHA 哈希函数
TEST(BigNumTest, ShaFunctions) {
    std::string message = "Hello, BigNum!";
    std::vector<uint8_t> data(message.begin(), message.end());
    
    // SHA-1
    auto sha1 = Sha1(std::span{data});
    EXPECT_EQ(sha1.size(), kSha1Size);
    
    // SHA-256
    auto sha256 = Sha256(std::span{data});
    EXPECT_EQ(sha256.size(), kSha256Size);
    
    // SHA-512
    auto sha512 = Sha512(std::span{data});
    EXPECT_EQ(sha512.size(), kSha512Size);
}

// 测试 HMAC
TEST(BigNumTest, Hmac) {
    std::string keyStr = "secret_key";
    std::string dataStr = "message_to_authenticate";
    
    std::vector<uint8_t> key(keyStr.begin(), keyStr.end());
    std::vector<uint8_t> data(dataStr.begin(), dataStr.end());
    
    auto hmac = HmacSha256(std::span{key}, std::span{data});
    EXPECT_EQ(hmac.size(), kSha256Size);
}

// 测试 PBKDF2
TEST(BigNumTest, Pbkdf2) {
    std::string password = "my_password";
    std::string salt = "random_salt";
    
    std::vector<uint8_t> pwdBytes(password.begin(), password.end());
    std::vector<uint8_t> saltBytes(salt.begin(), salt.end());
    
    int iterations = 10000;
    auto derived = Pbkdf2Sha512(std::span{pwdBytes}, std::span{saltBytes}, iterations);
    
    EXPECT_EQ(derived.size(), kSha512Size);
}

// 测试大数运算（RSA 场景）
TEST(BigNumTest, LargeNumbersRSA) {
    // 模拟 RSA 参数（简化版）
    BigNum p(61);      // 第一个质数
    BigNum q(53);      // 第二个质数
    BigNum n;
    n.setMul(p, q);    // n = p * q = 3233
    
    // 欧拉函数 φ(n) = (p-1) * (q-1)
    BigNum p_1 = BigNum::Sub(p, BigNum(1));
    BigNum q_1 = BigNum::Sub(q, BigNum(1));
    BigNum phi;
    phi.setMul(p_1, q_1);
    
    // 选择公钥指数 e = 17
    BigNum e(17);
    
    // 计算私钥 d = e^(-1) mod φ(n)
    Context ctx;
    BigNum d = BigNum::ModInverse(e, phi, ctx);
    EXPECT_FALSE(d.failed());
    
    // 测试加密/解密
    BigNum message(42);  // 明文消息
    
    // 加密: c = m^e mod n
    auto ciphertext = BigNum::ModExp(message, e, n, ctx);
    
    // 解密: m = c^d mod n
    auto decrypted = BigNum::ModExp(ciphertext, d, n, ctx);
    
    // 验证
    EXPECT_EQ(BigNum::Compare(message, decrypted), 0);
}

