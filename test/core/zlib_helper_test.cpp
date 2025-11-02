#include <gtest/gtest.h>
#include "utils/zlib_helper.hpp"

#include <string>
#include <vector>
#include <zlib.h>

namespace plib::core::utils
{
    TEST(ZlibHelperTest, RoundTripInMemoryZip)
    {
        const std::string entry_name = "payload.txt";
        const std::vector<uint8_t> payload = {'H', 'e', 'l', 'l', 'o', ' ', 'Z', 'i', 'p'};

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
}
