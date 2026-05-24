#pragma once

#include <emscripten/val.h>

template<typename TReader>
struct WorkData {
    TReader*         reader;
    int64_t          imageIdx;
    emscripten::val* onSuccess;
    emscripten::val* onError;

    ~WorkData() {
        delete onSuccess;
        delete onError;
    }
};
