#pragma once

#include <algorithm>
#include <cmath>
#include <numbers>
#include <random>

#include "bmp_processing.h"
#include "exceptions.h"

typedef std::vector<std::vector<double>> CoefficientsMatrix;

constexpr std::string_view kFilterCropName = "-crop";
constexpr std::string_view kFilterGrayscaleName = "-gs";
constexpr std::string_view kFilterNegativeName = "-neg";
constexpr std::string_view kFilterSharpeningName = "-sharp";
constexpr std::string_view kFilterEdgeDetectionName = "-edge";
constexpr std::string_view kFilterGaussianBlurName = "-blur";
constexpr std::string_view kFilterShuffleName = "-shuffle";

constexpr size_t kFilterCropParamsCount = 2;
constexpr size_t kFilterGrayscaleParamsCount = 0;
constexpr size_t kFilterNegativeParamsCount = 0;
constexpr size_t kFilterSharpeningParamsCount = 0;
constexpr size_t kFilterEdgeDetectionParamsCount = 1;
constexpr size_t kFilterGaussianBlurParamsCount = 1;
constexpr size_t kFilterShuffleParamsCount = 1;

constexpr double kRedToGray = 0.299;
constexpr double kGreenToGray = 0.587;
constexpr double kBlueToGray = 0.114;

const CoefficientsMatrix kFilterSharpeningMatrix = {{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}};
const CoefficientsMatrix kFilterEdgeDetectionMatrix = {{0, -1, 0}, {-1, 4, -1}, {0, -1, 0}};

constexpr double kMatrixSizeDependenceOnSigma = 3.0;
constexpr int kMinimumGaussianBlurMatrixSize = 5;
constexpr double kSigmaMultiplier = 2.0;

constexpr size_t kAmountOfSwappingPieces = 2;

class BaseFilter {
protected:
    std::string_view filter_name_;
    size_t required_params_count_;
    std::string invalid_arguments_message_;

    void CheckRightParamsCount(size_t params_count);

public:
    explicit BaseFilter(std::string_view filter_name, size_t required_params_count,
                        const std::vector<std::string>& params);

    virtual void Apply(BMP& image) = 0;

    virtual ~BaseFilter() = default;
};

class Crop : public BaseFilter {
    size_t width_;
    size_t height_;

    size_t ParseOrThrow(const std::string& argument);

public:
    explicit Crop(const std::vector<std::string>& params);

    void Apply(BMP& image) final;
};

class Grayscale : public BaseFilter {
public:
    explicit Grayscale(const std::vector<std::string>& params) : BaseFilter(kFilterGrayscaleName,
                                                                            kFilterGrayscaleParamsCount, params) {};

    void Apply(BMP& image) final;
};

class Negative : public BaseFilter {
public:
    explicit Negative(const std::vector<std::string>& params) : BaseFilter(kFilterNegativeName,
                                                                           kFilterNegativeParamsCount, params) {};

    void Apply(BMP& image) final;
};

class MatrixFilter {
protected:
    CoefficientsMatrix matrix_;

    MatrixFilter() = default;
    explicit MatrixFilter(CoefficientsMatrix matrix) : matrix_(std::move(matrix)) {};

    PixelColor CalculatePixel(const PixelMatrix& pixels, size_t pos_x, size_t pos_y, size_t matrix_size = 3);
};

class Sharpening : public BaseFilter, protected MatrixFilter {
public:
    explicit Sharpening(const std::vector<std::string>& params) : BaseFilter(kFilterSharpeningName,
                                                                             kFilterSharpeningParamsCount, params),
                                                                  MatrixFilter(kFilterSharpeningMatrix) {};

    void Apply(BMP& image) final;
};

class EdgeDetection : public BaseFilter, protected MatrixFilter {
    int threshold_{};

    void ParseOrThrow(const std::string& argument);

public:
    explicit EdgeDetection(const std::vector<std::string>& params);

    void Apply(BMP& image) final;
};

class GaussianBlur : public BaseFilter, protected MatrixFilter {
    double sigma_{};

    void ParseOrThrow(const std::string& argument);
    void CalculateGaussianMatrix();

public:
    explicit GaussianBlur(const std::vector<std::string>& params);

    void Apply(BMP& image) final;
};

class Shuffle : public BaseFilter {
    size_t pieces_on_one_side_{};
    size_t pieces_count_{};

    void ParseOrThrow(const std::string& argument);

public:
    explicit Shuffle(const std::vector<std::string>& params);

    void Apply(BMP& image) final;
};
