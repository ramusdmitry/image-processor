#pragma once

#include <string>
#include <vector>

#include "exceptions.h"

constexpr size_t kMinimalAmountOfArgs = 3;

struct Filter {
    std::string filter_name;
    std::vector<std::string> filter_params;
};

struct Arguments {
    std::string_view input_path;
    std::string_view output_path;

    std::vector<Filter> filters;
};

struct Parser {
    Arguments operator()(int argc, char **argv);
};
