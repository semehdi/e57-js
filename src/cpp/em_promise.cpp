#include <em_promise.hpp>

EM_JS(emscripten::EM_VAL, _emjs_make_deferred, (), {
    var resolve, reject;
    var promise = new Promise(function(res, rej) { resolve = res; reject = rej; });
    return Emval.toHandle({ promise: promise, resolve: resolve, reject: reject });
});

EM_JS(void, _emjs_call_resolve, (emscripten::EM_VAL resolve_fn, emscripten::EM_VAL value), {
    Emval.toValue(resolve_fn)(Emval.toValue(value));
});

EM_JS(void, _emjs_call_reject, (emscripten::EM_VAL reject_fn, const char* msg), {
    Emval.toValue(reject_fn)(new Error(UTF8ToString(msg)));
});

EM_JS(emscripten::EM_VAL, _emjs_array_view, (void* ptr, int32_t size, int32_t elementSize), {
    var arr = HEAPU8.subarray(ptr, ptr + size * elementSize);
    if (!globalThis._e57Finalizer)
        globalThis._e57Finalizer = new FinalizationRegistry(function(p) { _free(p); });
    globalThis._e57Finalizer.register(arr, ptr);
    return Emval.toHandle(arr);
});

EmPromise::EmPromise()
{
    emscripten::val deferred = emscripten::val::take_ownership(_emjs_make_deferred());
    _promise = deferred["promise"];
    _resolve = deferred["resolve"];
    _reject  = deferred["reject"];
}

emscripten::val EmPromise::take()
{
    return std::move(_promise);
}

void EmPromise::resolve(emscripten::val value)
{
    _emjs_call_resolve(_resolve.as_handle(), value.as_handle());
}

void EmPromise::reject(const std::string& message)
{
    _emjs_call_reject(_reject.as_handle(), message.c_str());
}

template <typename T>
emscripten::EM_VAL EmPromise::arrayView(T* ptr, int32_t size)
{
    return _emjs_array_view(ptr, size, sizeof(T));
}

template emscripten::EM_VAL EmPromise::arrayView<uint8_t>(uint8_t* ptr, int32_t size);
