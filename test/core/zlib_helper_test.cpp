#include <gtest/gtest.h>
#include "utils/zlib_helper.hpp"

#include <string>
#include <vector>
#include <zlib.h>

namespace plib::core::utils
{
    // 测试基本的 zip/unzip 往返
    TEST(ZlibHelperTest, RoundTripInMemoryZip)
    {
        const std::string entry_name = "payload.txt";
        const std::vector<uint8_t> payload = {'H', 'e', 'l', 'l', 'o', ' ', 'Z', 'i', 'p'};

        // 写入阶段
        InMemoryFile writer({});
        auto write_funcs = writer.funcs();
        write_funcs.opaque = &writer;

        zipFile zip = zipOpen2(nullptr, APPEND_STATUS_CREATE, nullptr, &write_funcs);
        ASSERT_NE(zip, nullptr);

        zip_fileinfo file_info{};
        int status = zipOpenNewFileInZip(zip,
                                         entry_name.c_str(),
                                         &file_info,
                                         nullptr,
                                         0,
                                         nullptr,
                                         0,
                                         nullptr,
                                         Z_DEFLATED,
                                         Z_DEFAULT_COMPRESSION);
        ASSERT_EQ(status, ZIP_OK);

        status = zipWriteInFileInZip(zip, payload.data(), static_cast<unsigned int>(payload.size()));
        ASSERT_EQ(status, ZIP_OK);
        ASSERT_EQ(zipCloseFileInZip(zip), ZIP_OK);
        ASSERT_EQ(zipClose(zip, nullptr), ZIP_OK);

        const std::vector<uint8_t> zipped_bytes = writer.result();
        ASSERT_FALSE(zipped_bytes.empty());

        // 读取阶段
        InMemoryFile reader(zipped_bytes);
        auto read_funcs = reader.funcs();
        read_funcs.opaque = &reader;

        unzFile unzip = unzOpen2(nullptr, &read_funcs);
        ASSERT_NE(unzip, nullptr);
        ASSERT_EQ(unzGoToFirstFile(unzip), UNZ_OK);

        char filename[64] = {};
        unz_file_info read_info{};
        ASSERT_EQ(unzGetCurrentFileInfo(unzip,
                                        &read_info,
                                        filename,
                                        sizeof(filename),
                                        nullptr,
                                        0,
                                        nullptr,
                                        0),
                  UNZ_OK);
        EXPECT_STREQ(filename, entry_name.c_str());

        ASSERT_EQ(unzOpenCurrentFile(unzip), UNZ_OK);
        std::vector<uint8_t> output(read_info.uncompressed_size);
        const int bytes_read = unzReadCurrentFile(unzip, output.data(), static_cast<unsigned int>(output.size()));
        ASSERT_EQ(bytes_read, static_cast<int>(payload.size()));
        EXPECT_EQ(output, payload);
        ASSERT_EQ(unzCloseCurrentFile(unzip), UNZ_OK);
        ASSERT_EQ(unzClose(unzip), UNZ_OK);
    }

    // 测试多个文件的 zip
    TEST(ZlibHelperTest, MultipleFilesInZip)
    {
        InMemoryFile writer({});
        auto write_funcs = writer.funcs();
        write_funcs.opaque = &writer;

        zipFile zip = zipOpen2(nullptr, APPEND_STATUS_CREATE, nullptr, &write_funcs);
        ASSERT_NE(zip, nullptr);

        // 添加第一个文件
        std::string file1_name = "file1.txt";
        std::vector<uint8_t> file1_data = {'F', 'i', 'l', 'e', '1'};
        zip_fileinfo file_info{};
        ASSERT_EQ(zipOpenNewFileInZip(zip, file1_name.c_str(), &file_info, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION), ZIP_OK);
        ASSERT_EQ(zipWriteInFileInZip(zip, file1_data.data(), static_cast<unsigned int>(file1_data.size())), ZIP_OK);
        ASSERT_EQ(zipCloseFileInZip(zip), ZIP_OK);

        // 添加第二个文件
        std::string file2_name = "file2.txt";
        std::vector<uint8_t> file2_data = {'F', 'i', 'l', 'e', '2'};
        ASSERT_EQ(zipOpenNewFileInZip(zip, file2_name.c_str(), &file_info, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION), ZIP_OK);
        ASSERT_EQ(zipWriteInFileInZip(zip, file2_data.data(), static_cast<unsigned int>(file2_data.size())), ZIP_OK);
        ASSERT_EQ(zipCloseFileInZip(zip), ZIP_OK);

        ASSERT_EQ(zipClose(zip, nullptr), ZIP_OK);

        // 读取并验证
        const std::vector<uint8_t> zipped_bytes = writer.result();
        InMemoryFile reader(zipped_bytes);
        auto read_funcs = reader.funcs();
        read_funcs.opaque = &reader;

        unzFile unzip = unzOpen2(nullptr, &read_funcs);
        ASSERT_NE(unzip, nullptr);

        // 验证第一个文件
        ASSERT_EQ(unzGoToFirstFile(unzip), UNZ_OK);
        char filename[64] = {};
        unz_file_info read_info{};
        ASSERT_EQ(unzGetCurrentFileInfo(unzip, &read_info, filename, sizeof(filename), nullptr, 0, nullptr, 0), UNZ_OK);
        EXPECT_STREQ(filename, file1_name.c_str());
        ASSERT_EQ(unzOpenCurrentFile(unzip), UNZ_OK);
        std::vector<uint8_t> output1(read_info.uncompressed_size);
        ASSERT_EQ(unzReadCurrentFile(unzip, output1.data(), static_cast<unsigned int>(output1.size())), static_cast<int>(file1_data.size()));
        EXPECT_EQ(output1, file1_data);
        ASSERT_EQ(unzCloseCurrentFile(unzip), UNZ_OK);

        // 验证第二个文件
        ASSERT_EQ(unzGoToNextFile(unzip), UNZ_OK);
        ASSERT_EQ(unzGetCurrentFileInfo(unzip, &read_info, filename, sizeof(filename), nullptr, 0, nullptr, 0), UNZ_OK);
        EXPECT_STREQ(filename, file2_name.c_str());
        ASSERT_EQ(unzOpenCurrentFile(unzip), UNZ_OK);
        std::vector<uint8_t> output2(read_info.uncompressed_size);
        ASSERT_EQ(unzReadCurrentFile(unzip, output2.data(), static_cast<unsigned int>(output2.size())), static_cast<int>(file2_data.size()));
        EXPECT_EQ(output2, file2_data);
        ASSERT_EQ(unzCloseCurrentFile(unzip), UNZ_OK);

        ASSERT_EQ(unzClose(unzip), UNZ_OK);
    }

    // 测试空文件
    TEST(ZlibHelperTest, EmptyFileInZip)
    {
        InMemoryFile writer({});
        auto write_funcs = writer.funcs();
        write_funcs.opaque = &writer;

        zipFile zip = zipOpen2(nullptr, APPEND_STATUS_CREATE, nullptr, &write_funcs);
        ASSERT_NE(zip, nullptr);

        std::string empty_name = "empty.txt";
        zip_fileinfo file_info{};
        ASSERT_EQ(zipOpenNewFileInZip(zip, empty_name.c_str(), &file_info, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION), ZIP_OK);
        // 不写入任何数据
        ASSERT_EQ(zipCloseFileInZip(zip), ZIP_OK);
        ASSERT_EQ(zipClose(zip, nullptr), ZIP_OK);

        // 读取并验证
        const std::vector<uint8_t> zipped_bytes = writer.result();
        InMemoryFile reader(zipped_bytes);
        auto read_funcs = reader.funcs();
        read_funcs.opaque = &reader;

        unzFile unzip = unzOpen2(nullptr, &read_funcs);
        ASSERT_NE(unzip, nullptr);
        ASSERT_EQ(unzGoToFirstFile(unzip), UNZ_OK);

        char filename[64] = {};
        unz_file_info read_info{};
        ASSERT_EQ(unzGetCurrentFileInfo(unzip, &read_info, filename, sizeof(filename), nullptr, 0, nullptr, 0), UNZ_OK);
        EXPECT_STREQ(filename, empty_name.c_str());
        EXPECT_EQ(read_info.uncompressed_size, 0);

        ASSERT_EQ(unzClose(unzip), UNZ_OK);
    }

    // 测试大文件
    TEST(ZlibHelperTest, LargeFileInZip)
    {
        const size_t large_size = 100000; // 100KB
        std::vector<uint8_t> large_data(large_size);
        for (size_t i = 0; i < large_size; ++i)
        {
            large_data[i] = static_cast<uint8_t>(i % 256);
        }

        InMemoryFile writer({});
        auto write_funcs = writer.funcs();
        write_funcs.opaque = &writer;

        zipFile zip = zipOpen2(nullptr, APPEND_STATUS_CREATE, nullptr, &write_funcs);
        ASSERT_NE(zip, nullptr);

        std::string large_name = "large.bin";
        zip_fileinfo file_info{};
        ASSERT_EQ(zipOpenNewFileInZip(zip, large_name.c_str(), &file_info, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_BEST_COMPRESSION), ZIP_OK);
        ASSERT_EQ(zipWriteInFileInZip(zip, large_data.data(), static_cast<unsigned int>(large_data.size())), ZIP_OK);
        ASSERT_EQ(zipCloseFileInZip(zip), ZIP_OK);
        ASSERT_EQ(zipClose(zip, nullptr), ZIP_OK);

        // 读取并验证
        const std::vector<uint8_t> zipped_bytes = writer.result();
        ASSERT_FALSE(zipped_bytes.empty());
        // 压缩后应该小于原始数据
        EXPECT_LT(zipped_bytes.size(), large_data.size());

        InMemoryFile reader(zipped_bytes);
        auto read_funcs = reader.funcs();
        read_funcs.opaque = &reader;

        unzFile unzip = unzOpen2(nullptr, &read_funcs);
        ASSERT_NE(unzip, nullptr);
        ASSERT_EQ(unzGoToFirstFile(unzip), UNZ_OK);

        unz_file_info read_info{};
        char filename[64] = {};
        ASSERT_EQ(unzGetCurrentFileInfo(unzip, &read_info, filename, sizeof(filename), nullptr, 0, nullptr, 0), UNZ_OK);
        EXPECT_EQ(read_info.uncompressed_size, large_size);

        ASSERT_EQ(unzOpenCurrentFile(unzip), UNZ_OK);
        std::vector<uint8_t> output(read_info.uncompressed_size);
        const int bytes_read = unzReadCurrentFile(unzip, output.data(), static_cast<unsigned int>(output.size()));
        ASSERT_EQ(bytes_read, static_cast<int>(large_size));
        EXPECT_EQ(output, large_data);
        ASSERT_EQ(unzCloseCurrentFile(unzip), UNZ_OK);
        ASSERT_EQ(unzClose(unzip), UNZ_OK);
    }

    // 测试 InMemoryFile 的初始状态
    TEST(ZlibHelperTest, InMemoryFileInitialState)
    {
        std::vector<uint8_t> initial_data = {1, 2, 3, 4, 5};
        InMemoryFile file(initial_data);

        EXPECT_EQ(file.error(), 0);
        EXPECT_EQ(file.result(), initial_data);
    }

    // 测试空数据初始化
    TEST(ZlibHelperTest, InMemoryFileEmptyInit)
    {
        InMemoryFile file({});
        EXPECT_EQ(file.error(), 0);
        EXPECT_TRUE(file.result().empty());
    }
}
