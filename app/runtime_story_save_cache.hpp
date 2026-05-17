#pragma once

#ifdef WHACKER_HAS_GLFW

namespace whacker::app {

using StorySaveExistsFn = bool (*)();

struct RuntimeStorySaveExistsCache {
    bool valid = false;
    bool value = false;
};

void invalidate_runtime_story_save_exists_cache(RuntimeStorySaveExistsCache& cache);
bool resolve_runtime_story_save_exists_cached(
    RuntimeStorySaveExistsCache& cache,
    StorySaveExistsFn exists_fn);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
