#include "runtime_story_save_cache.hpp"

#ifdef WHACKER_HAS_GLFW

namespace whacker::app {

void invalidate_runtime_story_save_exists_cache(RuntimeStorySaveExistsCache& cache) {
    cache.valid = false;
}

bool resolve_runtime_story_save_exists_cached(
    RuntimeStorySaveExistsCache& cache,
    const StorySaveExistsFn exists_fn) {
    if (!cache.valid) {
        cache.value = (exists_fn != nullptr) ? exists_fn() : false;
        cache.valid = true;
    }
    return cache.value;
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
