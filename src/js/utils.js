const isNode = typeof process !== 'undefined' && !!process.versions?.node;

/**
 * Keeps the Node.js event loop alive until `promise` settles.
 *
 * Async methods run on Emscripten pthreads, whose Workers are `unref()`'d in
 * Node so an idle thread pool doesn't block process exit. A side effect is
 * that a pending pthread-backed Promise holds no event-loop handle, so Node
 * may exit ("unsettled top-level await") before the worker posts its result.
 * A ref'd timer bridges that gap. No-op outside Node.
 *
 * @template T
 * @param {Promise<T>} promise
 * @returns {Promise<T>}
 */
export function keepAlive(promise)
{
    if (!isNode)
        return promise;
    const timer = setInterval(() => {}, 1 << 30);
    return promise.finally(() => clearInterval(timer));
}
