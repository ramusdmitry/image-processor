#pragma once

#include <memory>
#include <string>
#include <vector>

#include "bmp_processing.h"
#include "console_read.h"
#include "exceptions.h"
#include "filters.h"

enum class FiltersList : unsigned char {
    kNone,
    kCrop,
    kGrayscale,
    kNegative,
    kSharpening,
    kEdgeDetection,
    kGaussianBlur,
    kShuffle,
};

FiltersList GetFilter(const std::string& filter_name);

void ApplyFilters(const std::vector<Filter>& filters, BMP& image);
