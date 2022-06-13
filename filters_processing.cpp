#include "filters_processing.h"

FiltersList GetFilter(const std::string& filter_name) {
    if (filter_name == kFilterCropName) {
        return FiltersList::kCrop;
    } else if (filter_name == kFilterGrayscaleName) {
        return FiltersList::kGrayscale;
    } else if (filter_name == kFilterNegativeName) {
        return FiltersList::kNegative;
    } else if (filter_name == kFilterSharpeningName) {
        return FiltersList::kSharpening;
    } else if (filter_name == kFilterEdgeDetectionName) {
        return FiltersList::kEdgeDetection;
    } else if (filter_name == kFilterGaussianBlurName) {
        return FiltersList::kGaussianBlur;
    } else if (filter_name == kFilterShuffleName) {
        return FiltersList::kShuffle;
    }
    return FiltersList::kNone;
}

void ApplyFilters(const std::vector<Filter>& filters, BMP& image) {
    std::vector<std::shared_ptr<BaseFilter>> requested_filters;

    for (const auto& filter : filters) {
        switch (GetFilter(filter.filter_name)) {
            case FiltersList::kCrop: {
                requested_filters.push_back(std::make_shared<Crop>(filter.filter_params));
                continue;
            }
            case FiltersList::kGrayscale: {
                requested_filters.push_back(std::make_shared<Grayscale>(filter.filter_params));
                continue;
            }
            case FiltersList::kNegative: {
                requested_filters.push_back(std::make_shared<Negative>(filter.filter_params));
                continue;
            }
            case FiltersList::kSharpening: {
                requested_filters.push_back(std::make_shared<Sharpening>(filter.filter_params));
                continue;
            }
            case FiltersList::kEdgeDetection: {
                requested_filters.push_back(std::make_shared<EdgeDetection>(filter.filter_params));
                continue;
            }
            case FiltersList::kGaussianBlur: {
                requested_filters.push_back(std::make_shared<GaussianBlur>(filter.filter_params));
                continue;
            }
            case FiltersList::kShuffle: {
                requested_filters.push_back(std::make_shared<Shuffle>(filter.filter_params));
                continue;
            }
            default:
                throw FiltersProcessingException(filter.filter_name + " is not valid filter name");
        }
    }

    for (const auto& applied_filter : requested_filters) {
        applied_filter->Apply(image);
    }
}
