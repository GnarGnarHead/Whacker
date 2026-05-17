#include <cassert>

#include "runtime_story_save_cache.hpp"

namespace {

int g_story_save_exists_calls = 0;
bool g_story_save_exists_value = false;

bool stub_story_save_exists() {
    ++g_story_save_exists_calls;
    return g_story_save_exists_value;
}

void reset_stub(const bool value) {
    g_story_save_exists_calls = 0;
    g_story_save_exists_value = value;
}

void test_resolve_populates_cache_once() {
    reset_stub(true);
    whacker::app::RuntimeStorySaveExistsCache cache {};

    const bool first =
        whacker::app::resolve_runtime_story_save_exists_cached(cache, stub_story_save_exists);
    const bool second =
        whacker::app::resolve_runtime_story_save_exists_cached(cache, stub_story_save_exists);
    (void)first;
    (void)second;

    assert(first);
    assert(second);
    assert(g_story_save_exists_calls == 1);
}

void test_invalidate_forces_refetch() {
    reset_stub(false);
    whacker::app::RuntimeStorySaveExistsCache cache {};

    const bool first =
        whacker::app::resolve_runtime_story_save_exists_cached(cache, stub_story_save_exists);
    (void)first;
    assert(!first);
    assert(g_story_save_exists_calls == 1);

    whacker::app::invalidate_runtime_story_save_exists_cache(cache);
    g_story_save_exists_value = true;
    const bool second =
        whacker::app::resolve_runtime_story_save_exists_cached(cache, stub_story_save_exists);
    (void)second;
    assert(second);
    assert(g_story_save_exists_calls == 2);
}

void test_null_callback_defaults_to_false_without_calls() {
    reset_stub(true);
    whacker::app::RuntimeStorySaveExistsCache cache {};
    const bool value =
        whacker::app::resolve_runtime_story_save_exists_cached(cache, nullptr);
    (void)value;
    assert(!value);
    assert(g_story_save_exists_calls == 0);
}

}  // namespace

int main() {
    test_resolve_populates_cache_once();
    test_invalidate_forces_refetch();
    test_null_callback_defaults_to_false_without_calls();
    return 0;
}
