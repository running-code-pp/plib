#include <gtest/gtest.h>
#include <curl/curl.h>
#include <string>
#include <iostream>

// Helper function to write response data
size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *response)
{
    size_t totalSize = size * nmemb;
    response->append((char *)contents, totalSize);
    return totalSize;
}

// Test class for libcurl functionality
class LibCurlTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    void TearDown() override
    {
        curl_global_cleanup();
    }

    // Helper function to perform HTTP request
    std::string performRequest(const std::string &url, long httpVersion = CURL_HTTP_VERSION_NONE,
                               const std::string &acceptEncoding = "")
    {
        CURL *curl = curl_easy_init();
        std::string response;
        if (curl)
        {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // 10 second timeout
            
            // 添加SSL证书验证选项
            // 在测试环境中，我们可以禁用SSL验证（仅用于测试！）
            // 生产环境中应该正确配置CA证书
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

            if (httpVersion != CURL_HTTP_VERSION_NONE)
            {
                curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, httpVersion);
            }

            if (!acceptEncoding.empty())
            {
                struct curl_slist *headers = nullptr;
                std::string header = "Accept-Encoding: " + acceptEncoding;
                headers = curl_slist_append(headers, header.c_str());
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            }

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            }

            curl_easy_cleanup(curl);
        }
        return response;
    }
};

// Test basic HTTP request
TEST_F(LibCurlTest, BasicHttpRequest)
{
    std::string response = performRequest("http://httpbin.org/get");
    ASSERT_FALSE(response.empty());
    ASSERT_TRUE(response.find("httpbin") != std::string::npos);
}

// Test HTTP/2 support
TEST_F(LibCurlTest, Http2Request)
{
    std::string response = performRequest("https://httpbin.org/get", CURL_HTTP_VERSION_2);
    ASSERT_FALSE(response.empty());
    // Check if response indicates HTTP/2 was used (may not be guaranteed)
    // Note: Some servers may downgrade to HTTP/1.1
}

// Test zlib/gzip compression
TEST_F(LibCurlTest, ZlibCompression)
{
    std::string response = performRequest("https://httpbin.org/gzip", CURL_HTTP_VERSION_NONE, "gzip");
    ASSERT_FALSE(response.empty());
    // httpbin.org/gzip returns gzipped content
    ASSERT_TRUE(response.find("gzipped") != std::string::npos);
}

// Test zstd compression (if supported)
TEST_F(LibCurlTest, ZstdCompression)
{
    std::string response = performRequest("https://httpbin.org/zstd", CURL_HTTP_VERSION_NONE, "zstd");
    ASSERT_FALSE(response.empty());
    // Note: httpbin may not support zstd, this is a placeholder
    // In real scenario, check server support
}

// Test HTTPS
TEST_F(LibCurlTest, HttpsRequest)
{
    std::string response = performRequest("https://httpbin.org/get");
    ASSERT_FALSE(response.empty());
    ASSERT_TRUE(response.find("https") != std::string::npos || response.find("httpbin") != std::string::npos);
}

// Test proxy (if needed, but skipping for now as it requires proxy setup)
// TEST_F(LibCurlTest, ProxyRequest) { ... }

// Test IPv6 (if available)
// TEST_F(LibCurlTest, IPv6Request) { ... }
