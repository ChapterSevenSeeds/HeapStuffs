#include <cstdio>
#include <list>
#include <random>
#include <iostream>

#include "basic_heap.hpp"
#include "coalescing_heap.hpp"
#include "reuse_small_blocks_heap.hpp"

int main() {
    constexpr auto SIZE = 8192 * 128;
    basic_heap basic{SIZE};
    coalescing_heap coalescing{SIZE};
    reuse_small_blocks_heap reuse_small_blocks{SIZE};

    std::list<void *> basic_ptrs{};
    std::list<void *> coalescing_ptrs{};
    std::list<void *> reuse_small_blocks_ptrs{};

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> byte_dist(1, 1024);
    std::uniform_int_distribution<> should_free_dist(1, 5);
    while (true) {
        const auto bytes_to_allocate = byte_dist(gen);
        const auto should_free = should_free_dist(gen) > 3;

        auto basic_ptr = basic.alloc(bytes_to_allocate);
        auto coalescing_ptr = coalescing.alloc(bytes_to_allocate);
        auto reuse_small_blocks_ptr = reuse_small_blocks.alloc(bytes_to_allocate);

        if (basic_ptr == nullptr) {
            std::cout << "Failed basic allocation" << std::endl;
            break;
        }

        if (coalescing_ptr == nullptr) {
            std::cout << "Failed coalescing allocation" << std::endl;
            break;
        }

        if (reuse_small_blocks_ptr == nullptr) {
            std::cout << "Failed reuse_small_blocks allocation" << std::endl;
            break;
        }

        basic_ptrs.push_back(basic_ptr);
        coalescing_ptrs.push_back(coalescing_ptr);
        reuse_small_blocks_ptrs.push_back(reuse_small_blocks_ptr);

        if (should_free && !basic_ptrs.empty()) {
            basic.free(basic_ptrs.front());
            basic_ptrs.pop_front();

            coalescing.free(coalescing_ptrs.front());
            coalescing_ptrs.pop_front();

            reuse_small_blocks.free(reuse_small_blocks_ptrs.front());
            reuse_small_blocks_ptrs.pop_front();
        }
    }

    printf("BASIC: Fragmentation %f In use %f\n", basic.get_fragmentation(),
           static_cast<double>(basic.get_used_bytes()) / static_cast<double>(basic.original_size));
    printf("COALESCING: Fragmentation %f In use %f\n", coalescing.get_fragmentation(),
           static_cast<double>(coalescing.get_used_bytes()) / static_cast<double>(coalescing.original_size));
    printf("REUSE SMALL BLOCKS: Fragmentation %f In use %f\n", reuse_small_blocks.get_fragmentation(),
           static_cast<double>(reuse_small_blocks.get_used_bytes()) / static_cast<double>(reuse_small_blocks.
               original_size));
}
