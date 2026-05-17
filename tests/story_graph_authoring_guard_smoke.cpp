#include <cstdlib>

#include "story_script_catalog.hpp"

namespace {

void require(const bool condition) {
    if (!condition) {
        std::abort();
    }
}

}  // namespace

int main() {
    require(whacker::app::story_graph_all_authored_edges_resolve());
    return 0;
}
