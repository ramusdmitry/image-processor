#include "console_read.h"

Arguments Parser::operator()(int argc, char **argv) {
    if (argc < kMinimalAmountOfArgs) {
        throw ParserException("not enough params");
    }

    Arguments arguments{.input_path = argv[1], .output_path = argv[2]};

    auto arg = kMinimalAmountOfArgs;
    while (arg < argc) {
        if (argv[arg][0] != '-') {
            throw ParserException("wrong filters input (missing -)");
        }

        Filter filter;
        filter.filter_name = argv[arg];
        ++arg;
        while (arg < argc && argv[arg][0] != '-') {
            filter.filter_params.emplace_back(argv[arg]);
            ++arg;
        }
        arguments.filters.push_back(filter);
    }

    return arguments;
}
