#pragma once

#include <string>
#include <utility>

struct BaseException : public std::exception {
protected:
    std::string exception_message_;

public:
    explicit BaseException(std::string message) : exception_message_(std::move(message)) {};

    std::string what() {
        return exception_message_;
    }
};

struct ParserException : public BaseException {
    explicit ParserException(const std::string& message) : BaseException("Not valid console input: " + message) {};
};

struct FileProcessingException : public BaseException {
    explicit FileProcessingException(const std::string& message) : BaseException("File processing error: " +
                                                                                 message) {};
};

struct FiltersProcessingException : public BaseException {
    explicit FiltersProcessingException(const std::string& message) : BaseException("Filters processing error: " +
                                                                                    message) {};
};
