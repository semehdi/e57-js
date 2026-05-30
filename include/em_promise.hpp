#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include <emscripten.h>
#include <emscripten/val.h>


class EmPromise
{
public:
    EmPromise();

    emscripten::val take();
    void resolve(emscripten::val value);
    void reject(const std::string& message);
    
    template <typename T>
    static emscripten::EM_VAL arrayView(T* ptr, int32_t size);

private:
    emscripten::val _promise;
    emscripten::val _resolve;
    emscripten::val _reject;
};

