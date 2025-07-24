#include <cstdio>
#include <vector>
#include <ctime>

#include "basic_heap.hpp"
#include "coalescing_heap.hpp"

int main(void) {
    basic_heap basic{8192};
    coalescing_heap coalescing{8192};
    std::vector<void *> basic_ptrs{};
    std::vector<void *> coalescing_ptrs{};

    while (true) {
        std::srand(std::time({}));

        auto size_to_allocate = std::rand() % 128 + 1;
    }

    printf("Fragmentation %f", heap.get_fragmentation());
}
