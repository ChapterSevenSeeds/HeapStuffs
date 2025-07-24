#include <cstdio>
#include <list>
#include <random>
#include <iostream>

#include "basic_heap.hpp"
#include "coalescing_heap.hpp"

int main(void) {
    basic_heap basic{8192};
    coalescing_heap coalescing{8192};
    std::list<void *> basic_ptrs{};
    std::list<void *> coalescing_ptrs{};

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> byte_dist(1, 120);
    std::uniform_int_distribution<> should_free_dist(1, 5);
    while (true) {
        auto bytes_to_allocate = byte_dist(gen);
        auto should_free = should_free_dist(gen) > 3;

        auto basic_ptr = basic.alloc(bytes_to_allocate);
        auto coalescing_ptr = coalescing.alloc(bytes_to_allocate);

        if (basic_ptr == nullptr) {
            std::cout << "Failed basic allocation" << std::endl;
            break;
        }

        if (coalescing_ptr == nullptr) {
            std::cout << "Failed coalescing allocation" << std::endl;
            break;
        }

        basic_ptrs.push_back(basic_ptr);
        coalescing_ptrs.push_back(coalescing_ptr);

        if (should_free && !basic_ptrs.empty() && !coalescing_ptrs.empty()) {
            basic.free(basic_ptrs.front());
            basic_ptrs.pop_front();

            coalescing.free(coalescing_ptrs.front());
            coalescing_ptrs.pop_front();
        }
    }

    printf("BASIC: Fragmentation %f In use %llu Total %llu\n", basic.get_fragmentation(), basic.get_size(), basic.original_size);
    printf("COALESCING: Fragmentation %f In use %llu Total %llu\n", coalescing.get_fragmentation(), coalescing.get_size(), coalescing.original_size);
}
