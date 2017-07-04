#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <string>
#include <vector>
#include <fstream>

#include <iostream>

#include "zlib.h"

using namespace std;


// The difference in bytes between a zlib header and a gzip header.
const size_t kGzipZlibHeaderDifferenceBytes = 16;

// Pass an integer greater than the following get a gzip header instead of a
// zlib header when calling deflateInit2_.
const int kWindowBitsToGetGzipHeader = 16;

// This describes the amount of memory zlib uses to compress data. It can go
// from 1 to 9, with 8 being the default. For details, see:
// http://www.zlib.net/manual.html (search for memLevel).
const int kZlibMemoryLevel = 8;

// This code is taken almost verbatim from third_party/zlib/compress.c. The only
// difference is deflateInit2_ is called which sets the window bits to be > 16.
// That causes a gzip header to be emitted rather than a zlib header.
int GzipCompressHelper(Bytef* dest,
                       uLongf* dest_length,
                       const Bytef* source,
                       uLong source_length) {
    z_stream stream;

    stream.next_in = (Bytef*)(source);
    stream.avail_in = static_cast<uInt>(source_length);
    stream.next_out = dest;
    stream.avail_out = static_cast<uInt>(*dest_length);
    if (static_cast<uLong>(stream.avail_out) != *dest_length)
        return Z_BUF_ERROR;

    stream.zalloc = static_cast<alloc_func>(0);
    stream.zfree = static_cast<free_func>(0);
    stream.opaque = static_cast<voidpf>(0);

    gz_header gzip_header;
    memset(&gzip_header, 0, sizeof(gzip_header));
    int err = deflateInit2_(&stream,
                            Z_DEFAULT_COMPRESSION,
                            Z_DEFLATED,
                            MAX_WBITS + kWindowBitsToGetGzipHeader,
                            kZlibMemoryLevel,
                            Z_DEFAULT_STRATEGY,
                            ZLIB_VERSION,
                            sizeof(z_stream));
    if (err != Z_OK)
        return err;

    err = deflateSetHeader(&stream, &gzip_header);
    if (err != Z_OK)
        return err;

    err = deflate(&stream, Z_FINISH);
    if (err != Z_STREAM_END) {
        deflateEnd(&stream);
        return err == Z_OK ? Z_BUF_ERROR : err;
    }
    *dest_length = stream.total_out;

    err = deflateEnd(&stream);
    return err;
}

bool GzipCompress(const std::string& input, std::string* output) {
    std::vector<Bytef> compressed_data(kGzipZlibHeaderDifferenceBytes +
                                       compressBound(input.size()));

    uLongf compressed_size = compressed_data.size();
    if (GzipCompressHelper(&compressed_data.front(),
                           &compressed_size,
                           (const Bytef*)(input.data()),
                           input.size()) != Z_OK)
        return false;

    compressed_data.resize(compressed_size);
    output->assign(compressed_data.begin(), compressed_data.end());
    return true;
}

class GzipHelper
{
public:
    GzipHelper()
        : m_size(0)
    {

    }

    ~GzipHelper() {

    }

    int compressInit()
    {
        m_stream.zalloc = static_cast<alloc_func>(0);
        m_stream.zfree = static_cast<free_func>(0);
        m_stream.opaque = static_cast<voidpf>(0);

        int err = deflateInit2_(&m_stream,
                                Z_DEFAULT_COMPRESSION,
                                Z_DEFLATED,
                                MAX_WBITS + kWindowBitsToGetGzipHeader,
                                kZlibCompressLevel,
                                Z_DEFAULT_STRATEGY,
                                ZLIB_VERSION,
                                sizeof(z_stream));
        if (err != Z_OK)
            return err;

        m_data.reserve(compressBound((1 << 10)));

        return 0;
    }

    int compressUpdate(const uint8_t* source, uint32_t source_length)
    {
        m_stream.next_in = (Bytef*)(source);
        m_stream.avail_in = static_cast<uInt>(source_length);
        m_stream.next_out = &m_data.front() + m_size;
        m_stream.avail_out = static_cast<uInt>(m_data.capacity() - m_data.size());

        int err = deflate(&m_stream, Z_NO_FLUSH);
        if (err != Z_OK) {
            return err;
        }
        m_size = m_stream.total_out;
        return 0;
    }

    int compressFinish(const uint8_t* source, uint32_t source_length)
    {
        int err;
        m_stream.next_in = (Bytef*)(source);
        m_stream.avail_in = static_cast<uInt>(source_length);
        m_stream.next_out = &m_data.front() + m_size;
        m_stream.avail_out = static_cast<uInt>(m_data.capacity() - m_data.size());

        gz_header gzip_header;
        memset(&gzip_header, 0, sizeof(gzip_header));
        err = deflateSetHeader(&m_stream, &gzip_header);
        if (err != Z_OK) {
            cout << "deflateSetHeader  err = " << err << endl;
            return err;
        }

        err = deflate(&m_stream, Z_FINISH);
        if (err != Z_STREAM_END) {
            deflateEnd(&m_stream);
            return err == Z_OK ? Z_BUF_ERROR : err;
        }
        //deflateReset(stream);
        err = deflateEnd(&m_stream);
        if (err != Z_OK) {
            cout << "deflateSetHeader  err = " << err << endl;
            return err;
        }
        //m_data.resize(m_stream.total_out);
        m_size = m_stream.total_out;
        return 0;
    }

    uint64_t getCompressSize() {
        return m_size;
    }


    int dumpCompressFile(const char* path) 
    {
        int fd = open(path, O_CREAT | O_WRONLY | O_CLOEXEC, 0666);
        if (fd < 0) {
            return -1;
        }
        write(fd, &m_data.front(), m_size);
        fdatasync(fd);
        close(fd);
        return 0;
    }

public:
    static const size_t kGzipZlibHeaderDifferenceBytes = 16;
    static const int kWindowBitsToGetGzipHeader = 16;
    static const int kZlibCompressLevel = 8;
public:
    std::vector<Bytef> m_data;
    uint64_t m_size;
private:
    z_stream m_stream;
};

int main()
{
    //std::string in_str((100 << 20), '*');
    std::string in_str;
    std::string out_str;

    char line[1024];
    int i = 0;

    GzipHelper gzipHelper;

    std::string tst1("1: \n");
    std::string tst("");
    //gzipHelper.compressUpdate(tst.c_str(), tst.size());
    gzipHelper.compressInit();
    int err = gzipHelper.compressUpdate((uint8_t*)tst1.c_str(), tst1.size());
    if (err != 0) {
        cout << "err = " << err << endl;
    }

    tst1 = "2 \n";
    err = gzipHelper.compressUpdate((uint8_t*)tst1.c_str(), tst1.size());
    if (err != 0) {
        cout << "err = " << err << endl;
    }

    tst1 = "3 \n";
    err = gzipHelper.compressUpdate((uint8_t*)tst1.c_str(), tst1.size());
    if (err != 0) {
        cout << "err = " << err << endl;
    }

    cout << "getCompressSize = " << gzipHelper.getCompressSize()  << endl;

    err = gzipHelper.compressFinish((uint8_t*)tst.c_str(), tst.size());
    if (err != 0) {
        cout << "err = " << err << endl;
    }
    FILE* ff = fopen("1.gz", "w");
    if (ff) {
        cout << "m_data size " << gzipHelper.m_data.size() << endl;
        fwrite(&gzipHelper.m_data.front(), 1, gzipHelper.m_size, ff);
        fclose(ff);
    }

    cout << "getCompressSize = " << gzipHelper.getCompressSize()  << endl;


    std::ifstream fileStream("201706281202080000091014.txt");

    cout << fileStream.good() << endl;

    while (1) {
        fileStream.getline(line, 1024);
        if (fileStream.eof() || !fileStream.good()) break;
        in_str.append(line);
        in_str.append("\n");
        //cout << i++ << endl;
    }
    fileStream.close();

    printf("in_str.size = %u\n", in_str.size());

    GzipCompress(in_str, &out_str);
    FILE* f = fopen("test.gz", "w");
    if (f) {
        fwrite(out_str.c_str(), 1, out_str.size(), f);
        fclose(f);
    }

    return 0;
}

