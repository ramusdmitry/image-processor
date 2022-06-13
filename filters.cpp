#include "filters.h"

void BaseFilter::CheckRightParamsCount(size_t params_count) {
    if (params_count != required_params_count_) {
        throw FiltersProcessingException("wrong amount of params for filter " + std::string(filter_name_));
    }
}

BaseFilter::BaseFilter(std::string_view filter_name, size_t required_params_count,
                       const std::vector<std::string>& params) : filter_name_(filter_name),
                                                                 required_params_count_(required_params_count),
                                                                 invalid_arguments_message_(
                                                                         "wrong arguments for filter " +
                                                                         std::string(filter_name_)) {
    CheckRightParamsCount(params.size());
}

size_t Crop::ParseOrThrow(const std::string& argument) {
    try {
        auto converted_argument = std::stoull(argument);
        if (converted_argument == 0) {
            throw FiltersProcessingException(invalid_arguments_message_);
        }
        return converted_argument;
    } catch (std::logic_error& e) {
        throw FiltersProcessingException(invalid_arguments_message_);
    }
}

Crop::Crop(const std::vector<std::string>& params) : BaseFilter(kFilterCropName, kFilterCropParamsCount, params) {
    width_ = ParseOrThrow(params[0]);
    height_  = ParseOrThrow(params[1]);
}

void Crop::Apply(BMP& image) {
    if (height_ < image.GetHeight()) {
        image.ResizeHeight(height_);
    }

    if (width_ < image.GetWidth()) {
        image.ResizeWidth(width_);
    }
}

std::tuple<double, double, double> ConvertPixelToDouble(const PixelColor& pixel) {
    double red = static_cast<double>(pixel.r) / kMaxRgb;
    double green = static_cast<double>(pixel.g) / kMaxRgb;
    double blue = static_cast<double>(pixel.b) / kMaxRgb;

    return std::tie(red, green, blue);
}

void Grayscale::Apply(BMP& image) {
    for (auto& row : image.PixelMatrix()) {
        for (auto& pixel : row) {
            auto pixel_double = ConvertPixelToDouble(pixel);

            auto new_color = static_cast<uint8_t>(std::lround(((std::get<0>(pixel_double) * kRedToGray +
                                                                std::get<1>(pixel_double) * kGreenToGray +
                                                                std::get<2>(pixel_double) * kBlueToGray) *
                                                               kMaxRgb)));
            pixel.r = pixel.g = pixel.b = new_color;
        }
    }
}

void Negative::Apply(BMP& image) {
    for (auto& row : image.PixelMatrix()) {
        for (auto& pixel : row) {
            pixel.r = kMaxRgb - pixel.r;
            pixel.g = kMaxRgb - pixel.g;
            pixel.b = kMaxRgb - pixel.b;
        }
    }
}

PixelColor MatrixFilter::CalculatePixel(const PixelMatrix& pixels, size_t pos_y, size_t pos_x, size_t matrix_size) {
    double red = kMinRgb;
    double green = kMinRgb;
    double blue = kMinRgb;
    auto radius = static_cast<long long>((matrix_size - 1) / 2);
    size_t y, x;

    for (auto y_diff = -radius; y_diff <= radius; ++y_diff) {
        for (auto x_diff = -radius; x_diff <= radius; ++x_diff) {
            if ((pos_x + x_diff == 0 || pos_x + x_diff > pixels[0].size()) &&
                (pos_y + y_diff == 0 || pos_y + y_diff > pixels.size())) {
                y = pos_y - 1;
                x = pos_x - 1;
            } else if (pos_x + x_diff == 0 || pos_x + x_diff > pixels[0].size()) {
                y = pos_y + y_diff - 1;
                x = pos_x - 1;
            } else if (pos_y + y_diff == 0 || pos_y + y_diff > pixels.size()) {
                y = pos_y - 1;
                x = pos_x + x_diff - 1;
            } else {
                y = pos_y + y_diff - 1;
                x = pos_x + x_diff - 1;
            }

            auto pixel_double = ConvertPixelToDouble(pixels[y][x]);
            red += std::get<0>(pixel_double) * matrix_[y_diff + radius][x_diff + radius];
            green += std::get<1>(pixel_double) * matrix_[y_diff + radius][x_diff + radius];
            blue += std::get<2>(pixel_double) * matrix_[y_diff + radius][x_diff + radius];
        }
    }

    PixelColor new_pixel;
    new_pixel.r = std::clamp(static_cast<int>(std::lround(red * kMaxRgb)), kMinRgb, kMaxRgb);
    new_pixel.g = std::clamp(static_cast<int>(std::lround(green * kMaxRgb)), kMinRgb, kMaxRgb);
    new_pixel.b = std::clamp(static_cast<int>(std::lround(blue * kMaxRgb)), kMinRgb, kMaxRgb);
    return new_pixel;
}

void Sharpening::Apply(BMP& image) {
    const PixelMatrix read_only_pixels = image.PixelMatrix();
    for (auto row_number = image.GetHeight(); row_number > 0; --row_number) {
        for (auto col_number = image.GetWidth(); col_number > 0; --col_number) {
            image.PixelMatrix()[row_number - 1][col_number - 1] = CalculatePixel(read_only_pixels, row_number,
                                                                                 col_number);
        }
    }
}

void EdgeDetection::ParseOrThrow(const std::string& argument) {
    try {
        threshold_ = std::stoi(argument);
        if (threshold_ > kMaxRgb) {
            throw FiltersProcessingException(invalid_arguments_message_);
        }
    } catch (std::logic_error& e) {
        throw FiltersProcessingException(invalid_arguments_message_);
    }
}

EdgeDetection::EdgeDetection(const std::vector<std::string>& params) : BaseFilter(kFilterEdgeDetectionName,
                                                                                  kFilterEdgeDetectionParamsCount,
                                                                                  params),
                                                                       MatrixFilter(kFilterEdgeDetectionMatrix) {
    ParseOrThrow(params[0]);
}

void EdgeDetection::Apply(BMP& image) {
    Grayscale({}).Apply(image);

    const PixelMatrix read_only_pixels = image.PixelMatrix();
    for (auto row_number = image.GetHeight(); row_number > 0; --row_number) {
        for (auto col_number = image.GetWidth(); col_number > 0; --col_number) {
            if (CalculatePixel(read_only_pixels, row_number, col_number).r > threshold_) {
                image.PixelMatrix()[row_number - 1][col_number - 1] = {kMaxRgb, kMaxRgb, kMaxRgb};
            } else {
                image.PixelMatrix()[row_number - 1][col_number - 1] = {kMinRgb, kMinRgb, kMinRgb};
            }
        }
    }
}

void GaussianBlur::CalculateGaussianMatrix() {
    int size = std::max(kMinimumGaussianBlurMatrixSize,
                        static_cast<int>(std::lround((kMatrixSizeDependenceOnSigma * sigma_))));
    if (size % 2 == 0) {
        --size;
    }
    matrix_.resize(size);

    double matrix_center = static_cast<double>(size) / 2;
    double distance_y;
    double distance_x;
    double sigma_coefficient = kSigmaMultiplier * sigma_ * sigma_;
    double sum = 0;

    for (auto y = 0; y < size; ++y) {
        distance_y = matrix_center - y;
        matrix_[y].resize(size);

        for (auto x = 0; x < size; ++x) {
            distance_x = matrix_center - x;

            matrix_[y][x] = exp(-(distance_y * distance_y + distance_x * distance_x) / sigma_coefficient) /
                            (std::numbers::pi_v<double> * sigma_coefficient);
            sum += matrix_[y][x];
        }
    }

    for (auto y = 0; y < size; ++y) {
        for (auto x = 0; x < size; ++x) {
            matrix_[y][x] /= sum;
        }
    }
}

void GaussianBlur::ParseOrThrow(const std::string& argument) {
    try {
        sigma_ = std::stod(argument);
    } catch (std::logic_error& e) {
        throw FiltersProcessingException(invalid_arguments_message_);
    }
}

GaussianBlur::GaussianBlur(const std::vector<std::string>& params) : BaseFilter(kFilterGaussianBlurName,
                                                                                kFilterGaussianBlurParamsCount,
                                                                                params) {
    ParseOrThrow(params[0]);
    CalculateGaussianMatrix();
}

void GaussianBlur::Apply(BMP& image) {
    const PixelMatrix read_only_pixels = image.PixelMatrix();
    for (auto row_number = image.GetHeight(); row_number > 0; --row_number) {
        for (auto col_number = image.GetWidth(); col_number > 0; --col_number) {
            image.PixelMatrix()[row_number - 1][col_number - 1] = CalculatePixel(read_only_pixels, row_number,
                                                                                 col_number, matrix_.size());
        }
    }
}

void Shuffle::ParseOrThrow(const std::string& argument) {
    try {
        pieces_count_ = std::stoull(argument);
        pieces_on_one_side_ = static_cast<size_t>(std::sqrt(pieces_count_));
        if (pieces_count_ == 0 || pieces_on_one_side_ * pieces_on_one_side_ != pieces_count_) {
            throw FiltersProcessingException(invalid_arguments_message_);
        }
    } catch (std::logic_error& e) {
        throw FiltersProcessingException(invalid_arguments_message_);
    }
}

Shuffle::Shuffle(const std::vector<std::string>& params) : BaseFilter(kFilterShuffleName,
                                                                      kFilterShuffleParamsCount, params) {
    ParseOrThrow(params[0]);
}

void Shuffle::Apply(BMP& image) {
    if (pieces_on_one_side_ >= std::min(image.GetHeight(), image.GetWidth())) {
        return;
    }

    Crop({std::to_string(image.GetHeight() - image.GetHeight() % pieces_on_one_side_),
          std::to_string(image.GetWidth() - image.GetWidth() % pieces_on_one_side_)}).Apply(image);

    size_t piece_height = image.GetHeight() / pieces_on_one_side_;
    size_t piece_width = image.GetWidth() / pieces_on_one_side_;

    std::vector<std::tuple<size_t, size_t>> pieces_starts;
    for (size_t pos_y = 0; pos_y < image.GetHeight(); pos_y += piece_height) {
        for (size_t pos_x = 0; pos_x < image.GetWidth(); pos_x += piece_width) {
            pieces_starts.emplace_back(std::tie(pos_y, pos_x));
        }
    }
    std::shuffle(pieces_starts.begin(), pieces_starts.end(), std::random_device());

    for (auto piece_number = 0; piece_number < pieces_count_; piece_number += kAmountOfSwappingPieces) {
        if (pieces_count_ - piece_number < kAmountOfSwappingPieces) {
            break;
        }

        size_t first_piece_y = std::get<0>(pieces_starts[piece_number]);
        size_t first_piece_x = std::get<1>(pieces_starts[piece_number]);
        size_t second_piece_y = std::get<0>(pieces_starts[piece_number + 1]);
        size_t second_piece_x = std::get<1>(pieces_starts[piece_number + 1]);

        for (auto y_range = 0; y_range < piece_height; ++y_range) {
            for (auto x_range = 0; x_range < piece_width; ++x_range) {
                auto tmp = image.PixelMatrix()[first_piece_y + y_range][first_piece_x + x_range];
                image.PixelMatrix()[first_piece_y + y_range][first_piece_x + x_range] =
                        image.PixelMatrix()[second_piece_y + y_range][second_piece_x + x_range];
                image.PixelMatrix()[second_piece_y + y_range][second_piece_x + x_range] = tmp;
            }
        }
    }
}
