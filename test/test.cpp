#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "..\bmp_processing.h"
#include "..\console_read.h"
#include "..\exceptions.h"
#include "..\filters.h"
#include "..\filters_processing.h"

void CheckMatricesEquality(const PixelMatrix& gotten, const PixelMatrix& expected) {
    REQUIRE(gotten.size() == expected.size());
    for (auto row_number = expected.size() - 1; row_number > 0; --row_number) {
        REQUIRE(gotten[row_number].size() == expected[row_number].size());
        for (auto col_number = expected[0].size() - 1; col_number > 0; --col_number) {
            REQUIRE(gotten[row_number][col_number].r == expected[row_number][col_number].r);
            REQUIRE(gotten[row_number][col_number].g == expected[row_number][col_number].g);
            REQUIRE(gotten[row_number][col_number].b == expected[row_number][col_number].b);
        }
    }
}

TEST_CASE("ConsoleRead") {
    {
        Parser parser;

        const char* test_arguments[] = {".\\image_processor", ".\\examples\\example.bmp"};
        std::string expected_error = "not enough params";

        bool got_right_exception = false;

        try {
            auto args = parser(kMinimalAmountOfArgs - 1, const_cast<char**>(test_arguments));
        } catch (ParserException &e) {
            REQUIRE(e.what() == "Not valid console input: " + expected_error);
            got_right_exception = true;
        }

        if (!got_right_exception) {
            FAIL("Expected ParserException for " + expected_error);
        }
    }
    {
        Parser parser;

        constexpr size_t filters_count = 1;
        const char* test_arguments[] = {".\\image_processor", ".\\examples\\example.bmp",
                                        ".\\output\\program_test.bmp", "gs"};

        bool got_right_exception = false;
        std::string expected_error =  "wrong filters input (missing -)";

        try {
            auto args = parser(kMinimalAmountOfArgs + filters_count, const_cast<char**>(test_arguments));
        } catch (ParserException& e) {
            REQUIRE(e.what() == "Not valid console input: " + expected_error);
            got_right_exception = true;
        }

        if (!got_right_exception) {
            FAIL("Expected ParserException for " + expected_error);
        }
    }
    {
        Parser parser;

        constexpr size_t filters_count = 4;
        const char* test_arguments[] = {".\\image_processor", ".\\examples\\example.bmp",
                                        ".\\output\\program_test.bmp", "-crop", "800", "600", "-gs"};

        auto args = parser(kMinimalAmountOfArgs + filters_count, const_cast<char**>(test_arguments));
        REQUIRE(args.input_path == ".\\examples\\example.bmp");
        REQUIRE(args.output_path == ".\\output\\program_test.bmp");
        REQUIRE(args.filters[0].filter_name == "-crop");
        REQUIRE(args.filters[0].filter_params[0] == "800");
        REQUIRE(args.filters[0].filter_params[1] == "600");
        REQUIRE(args.filters[1].filter_name == "-gs");
        REQUIRE(args.filters.size() == 2);
    }
}

TEST_CASE("FIleProcessing") {
    {
        BMP image;

        const char input_path[] = ".\\Nan";

        bool got_right_exception = false;
        std::string expected_error =  "can not open for reading .\\Nan";

        try {
            image.Open(input_path);
        } catch (FileProcessingException &e) {
            REQUIRE(e.what() == "File processing error: " + expected_error);
            got_right_exception = true;
        }

        if (!got_right_exception) {
            FAIL("Expected FileProcessingException for " + expected_error);
        }
    }
    {
        BMP image;

        const char input_path[] = R"(..\..\test\test.txt)";

        bool got_right_exception = false;
        std::string expected_error =  R"(invalid input file ..\..\test\test.txt)";

        try {
            image.Open(input_path);
        } catch (FileProcessingException &e) {
            REQUIRE(e.what() == "File processing error: " + expected_error);
            got_right_exception = true;
        }

        if (!got_right_exception) {
            FAIL("Expected FileProcessingException for " + expected_error);
        }
    }
    {
        BMP image;

        const char input_path[] = R"(..\..\examples\example.bmp)";
        const char output_path[] = R"(..\..\output\testing_program.bmp)";

        image.Open(input_path);
        image.Save(output_path);
    }
}

TEST_CASE("FiltersProcessing") {
    {
        BMP image;

        bool got_right_exception = false;
        std::string expected_error = "wrong amount of params for filter -crop";

        try {
            ApplyFilters(std::vector<Filter>{Filter{.filter_name = "-crop"}}, image);
        } catch (FiltersProcessingException& e) {
            REQUIRE(e.what() == "Filters processing error: " + expected_error);
            got_right_exception = true;
        }

        if (!got_right_exception) {
            FAIL("Expected FiltersProcessingException for " + expected_error);
        }
    }
    {
        BMP image;

        bool got_right_exception = false;
        std::string expected_error = "-gummy is not valid filter name";

        try {
            ApplyFilters(std::vector<Filter>{Filter{.filter_name = "-gummy"}}, image);
        } catch (FiltersProcessingException &e) {
            REQUIRE(e.what() == "Filters processing error: " + expected_error);
            got_right_exception = true;
        }

        if (!got_right_exception) {
            FAIL("Expected FiltersProcessingException for " + expected_error);
        }
    }
    {
        BMP image;

        bool got_right_exception = false;
        std::string expected_error = "wrong arguments for filter -crop";

        try {
            ApplyFilters(std::vector<Filter>{Filter{.filter_name = "-crop", .filter_params = {"0", "0"}}}, image);
        } catch (FiltersProcessingException& e) {
            REQUIRE(e.what() == "Filters processing error: " + expected_error);
            got_right_exception = true;
        }

        if (!got_right_exception) {
            FAIL("Expected FiltersProcessingException for " + expected_error);
        }
    }
}

TEST_CASE("FilterCrop") {
    {
        BMP image;
        image.ResizeHeight(3);
        image.ResizeWidth(3);
        PixelMatrix pixels(3, std::vector<PixelColor>(3, {5, 5, 5}));
        image.PixelMatrix() = pixels;

        Crop({"1", "2"}).Apply(image);

        REQUIRE(image.PixelMatrix().size() == 2);
        REQUIRE(image.GetHeight() == 2);
        REQUIRE(image.PixelMatrix()[0].size() == 1);
        REQUIRE(image.GetWidth() == 1);
    }
    {
        BMP image;
        image.ResizeHeight(3);
        image.ResizeWidth(3);
        PixelMatrix pixels(3, std::vector<PixelColor>(3, {5, 5, 5}));
        image.PixelMatrix() = pixels;

        Crop({"5", "5"}).Apply(image);

        REQUIRE(image.PixelMatrix().size() == 3);
        REQUIRE(image.GetHeight() == 3);
        REQUIRE(image.PixelMatrix()[0].size() == 3);
        REQUIRE(image.GetWidth() == 3);
    }
}

TEST_CASE("FilterGrayscale") {
    {
        BMP image;
        image.ResizeHeight(3);
        image.ResizeWidth(3);
        PixelMatrix pixels(3, std::vector<PixelColor>(3, {5, 5, 5}));
        image.PixelMatrix() = pixels;

        Grayscale({}).Apply(image);

        PixelMatrix expected(3, std::vector<PixelColor>(3, {5, 5, 5}));

        CheckMatricesEquality(image.PixelMatrix(), expected);
    }
}

TEST_CASE("FilterNegative") {
    {
        BMP image;
        image.ResizeHeight(3);
        image.ResizeWidth(3);
        PixelMatrix pixels(3, std::vector<PixelColor>(3, {5, 10, 100}));
        image.PixelMatrix() = pixels;

        Negative({}).Apply(image);

        PixelMatrix expected(3, std::vector<PixelColor>(3, {250, 245, 155}));

        CheckMatricesEquality(image.PixelMatrix(), expected);
    }
}

TEST_CASE("FilterSharpening") {
    {
        BMP image;
        image.ResizeHeight(3);
        image.ResizeWidth(3);
        PixelMatrix pixels(3, std::vector<PixelColor>(3, {5, 5, 5}));
        pixels[1][1] = {255, 255, 255};
        pixels[0][1] = {8, 8, 8};
        image.PixelMatrix() = pixels;

        Sharpening({}).Apply(image);

        PixelMatrix expected(3, std::vector<PixelColor>(3, {0, 0, 0}));
        expected[0][0] = {2, 2, 2};
        expected[0][2] = {2, 2, 2};
        expected[1][1] = {255, 255, 255};
        expected[2][0] = {5, 5, 5};
        expected[2][2] = {5, 5, 5};

        CheckMatricesEquality(image.PixelMatrix(), expected);
    }
}

TEST_CASE("FilterGaussianBlur") {
    {
        BMP image;
        image.ResizeHeight(3);
        image.ResizeWidth(3);
        PixelMatrix pixels(3, std::vector<PixelColor>(3, {100, 100, 100}));
        pixels[0][0].r = 200;
        pixels[0][1].r = 150;
        image.PixelMatrix() = pixels;

        GaussianBlur({"1"}).Apply(image);

        PixelMatrix expected(3, std::vector<PixelColor>(3, {100, 100, 100}));
        expected[0][0].r = 135;
        expected[1][0].r = 109;
        expected[2][0].r = 101;
        expected[0][1].r = 120;
        expected[1][1].r = 105;
        expected[2][1].r = 101;
        expected[0][2].r = 104;
        expected[1][2].r = 101;
        expected[2][2].r = 100;

        CheckMatricesEquality(image.PixelMatrix(), expected);
    }
}
