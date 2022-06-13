#include <iostream>

#include "bmp_processing.h"
#include "console_read.h"
#include "exceptions.h"
#include "filters_processing.h"

int main(int argc, char* argv[]) {
    BMP image;
    Parser parser;
    try {
        auto args = parser(argc, argv);
        image.Open(args.input_path);
        ApplyFilters(args.filters, image);
        image.Save(args.output_path);
    } catch (BaseException& e) {
        std::cout << e.what() << std::endl;
    }
}
