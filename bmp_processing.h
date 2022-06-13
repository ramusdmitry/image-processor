#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "exceptions.h"

typedef unsigned char Byte;
typedef unsigned int Dword;
typedef unsigned short int Word;
typedef signed int Llong;

constexpr size_t kBmpMagicBytesCount = 2;
constexpr Byte kBmpSignatureFirstByte = 0x42;
constexpr Byte kBmpSignatureSecondByte = 0x4D;

constexpr size_t kBmpFileHeaderBytesCount = 12;
constexpr size_t kBmpInfoHeaderBytesCount = 40;

constexpr Word kRequiredBitsPerPixel = 24;
constexpr Dword kRequiredCompression = 0;
constexpr Llong kPadding = 4;
constexpr size_t kAmountOfPrimaryColors = 3;

constexpr int kMinRgb = 0;
constexpr int kMaxRgb = 255;

struct BitmapFileHeader {
    Dword file_size;
    Word reserved1;
    Word reserved2;
    Dword offset;
};

struct BitmapInfo {
    Dword header_size;
    Llong width;
    Llong height;
    Word planes;
    Word bits_per_pixel;
    Dword compression;
    Dword size_image;
    Llong h_res;
    Llong v_res;
    Dword num_colors;
    Dword num_important_colors;
};

struct PixelColor {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

typedef std::vector<std::vector<PixelColor>> PixelMatrix;

class BMP {
private:
    Byte magic_[kBmpMagicBytesCount];
    BitmapFileHeader file_header_;
    BitmapInfo info_;
    PixelMatrix pixels_;

public:
    void ReadMagic(std::ifstream& in, std::string_view input_file);
    void ReadFileHeader(std::ifstream& in, std::string_view input_file);
    void ReadInfo(std::ifstream& in, std::string_view input_file);
    void ReadHeaders(std::ifstream& in, std::string_view input_file);
    static PixelColor ReadPixel(std::ifstream& in, std::string_view input_file);
    void ReadImage(std::ifstream& in, std::string_view input_file);

    void WriteMagic(std::ofstream& out);
    void WriteFileHeader(std::ofstream& out);
    void WriteInfo(std::ofstream& out);
    static void WritePixel(std::ofstream& out, PixelColor pixel);
    void WriteImage(std::ofstream& out);
    void WriteHeaders(std::ofstream& out);

    void Open(std::string_view input_file);
    void Save(std::string_view output_file);

    PixelMatrix& PixelMatrix();

    size_t GetHeight() const;
    size_t GetWidth() const;
    void ResizeHeight(size_t height);
    void ResizeWidth(size_t width);
};
