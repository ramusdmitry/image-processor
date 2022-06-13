#include "bmp_processing.h"

void BMP::ReadMagic(std::ifstream& in, std::string_view input_file) {
    if (!in.read(reinterpret_cast<char*>(&magic_), kBmpMagicBytesCount)) {
        throw FileProcessingException("invalid input file " + std::string(input_file));
    }

    if (magic_[0] != kBmpSignatureFirstByte || magic_[1] != kBmpSignatureSecondByte) {
        throw FileProcessingException(std::string(input_file) + " is not right BMP format");
    }
}

void BMP::ReadFileHeader(std::ifstream& in, std::string_view input_file) {
    if (!in.read(reinterpret_cast<char*>(&file_header_), kBmpFileHeaderBytesCount)) {
        throw FileProcessingException("can not read Bitmap file header from " + std::string(input_file));
    }
}

void BMP::ReadInfo(std::ifstream& in, std::string_view input_file) {
    if (!in.read(reinterpret_cast<char*>(&info_), kBmpInfoHeaderBytesCount)) {
        throw FileProcessingException("can not read Bitmap info from " + std::string(input_file));
    }

    if (info_.bits_per_pixel != kRequiredBitsPerPixel) {
        throw FileProcessingException(std::string(input_file) + " is not 24 bits per pixel");
    }

    if (info_.compression != kRequiredCompression) {
        throw FileProcessingException(std::string(input_file) + " is compressed");
    }

    if (info_.height == 0 || info_.width <= 0) {
        throw FileProcessingException(std::string(input_file) + " have invalid height or width");
    }
}

void BMP::ReadHeaders(std::ifstream& in, std::string_view input_file) {
    ReadMagic(in, input_file);
    ReadFileHeader(in, input_file);
    ReadInfo(in, input_file);
}

PixelColor BMP::ReadPixel(std::ifstream& in, std::string_view input_file) {
    PixelColor pixel;
    pixel.b = in.get();
    pixel.g = in.get();
    pixel.r = in.get();

    if (pixel.b > kMaxRgb || pixel.b < kMinRgb ||
        pixel.g > kMaxRgb || pixel.g < kMinRgb ||
        pixel.r > kMaxRgb || pixel.r < kMinRgb) {
        throw FileProcessingException(std::string(input_file) + " have invalid pixels");
    }

    return pixel;
}

void BMP::ReadImage(std::ifstream& in, std::string_view input_file) {
    bool put_pixels_at_top = true;
    if (info_.height < 0) {
        put_pixels_at_top = false;
        info_.height = -info_.height;
    }

    in.seekg(file_header_.offset);

    for (auto row_number = 0; row_number < info_.height; ++row_number) {
        std::vector<PixelColor> row;

        row.reserve(info_.width);
        for (auto col_number = 0; col_number < info_.width; ++col_number) {
            row.push_back(ReadPixel(in, input_file));
        }

        in.seekg(info_.width % kPadding, std::ios::cur);

        if (put_pixels_at_top) {
            pixels_.insert(pixels_.begin(), row);
        } else {
            pixels_.push_back(row);
        }
    }
}

void BMP::Open(std::string_view input_file) {
    std::ifstream in(input_file.data(), std::ios::in | std::ios::binary);

    if (!in) {
        throw FileProcessingException("can not open for reading " + std::string(input_file));
    }

    ReadHeaders(in, input_file);
    ReadImage(in, input_file);

    in.close();
}

void BMP::WriteMagic(std::ofstream& out) {
    out.write(reinterpret_cast<char*>(&magic_), kBmpMagicBytesCount);
}

void BMP::WriteFileHeader(std::ofstream& out) {
    file_header_.offset = kBmpMagicBytesCount + kBmpFileHeaderBytesCount + kBmpInfoHeaderBytesCount;
    file_header_.file_size = file_header_.offset + (GetHeight() * kAmountOfPrimaryColors +
                                                    GetWidth() % kPadding) * GetHeight();

    out.write(reinterpret_cast<char*>(&file_header_), kBmpFileHeaderBytesCount);
}

void BMP::WriteInfo(std::ofstream& out) {
    out.write(reinterpret_cast<char*>(&info_), kBmpInfoHeaderBytesCount);
}

void BMP::WriteHeaders(std::ofstream& out) {
    WriteMagic(out);
    WriteFileHeader(out);
    WriteInfo(out);
}

void BMP::WritePixel(std::ofstream& out, PixelColor pixel) {
    out.put(static_cast<char>(pixel.b));
    out.put(static_cast<char>(pixel.g));
    out.put(static_cast<char>(pixel.r));
}

void BMP::WriteImage(std::ofstream& out) {
    for (auto row_number = GetHeight(); row_number > 0; --row_number) {
        const std::vector<PixelColor>& row = pixels_[row_number - 1];

        for (auto pixel : row) {
            WritePixel(out, pixel);
        }

        for (auto padding_byte = 0; padding_byte < GetWidth() % kPadding; ++padding_byte) {
            out.put(0);
        }
    }
}

void BMP::Save(std::string_view output_file) {
    std::ofstream out(output_file.data(), std::ios::out | std::ios::binary);

    if (!out) {
        throw FileProcessingException("can not open for editing " + std::string(output_file));
    }

    WriteHeaders(out);
    WriteImage(out);

    out.close();
}

PixelMatrix& BMP::PixelMatrix() {
    return pixels_;
}

size_t BMP::GetHeight() const {
    return info_.height;
}

size_t BMP::GetWidth() const {
    return info_.width;
}

void BMP::ResizeHeight(size_t height) {
    pixels_.resize(height);
    info_.height = static_cast<Llong>(height);
}

void BMP::ResizeWidth(size_t width) {
    for (auto& row : pixels_) {
        row.resize(width);
    }
    info_.width = static_cast<Llong>(width);
}
