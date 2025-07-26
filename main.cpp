#include <cstdio>
#include <list>
#include <random>
#include <iostream>

#include "basic_heap.hpp"
#include "coalescing_heap.hpp"

int main() {
    constexpr auto SIZE = 8192 * 128;
    basic_heap basic{SIZE};
    coalescing_heap coalescing{SIZE};

    std::vector<void *> basic_ptrs{};
    std::vector<void *> coalescing_ptrs{};

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> byte_dist(1, static_cast<int>(std::sqrt(SIZE)));
    std::uniform_int_distribution<> should_free_dist(1, 5);
    size_t loop_count = 0;
    while (true) {
        loop_count++;

        const auto bytes_to_allocate = byte_dist(gen);
        const auto should_free = should_free_dist(gen) > 1;

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

        if (should_free && !basic_ptrs.empty()) {
            std::uniform_int_distribution<> index_to_free_dist(0, static_cast<int>(basic_ptrs.size() - 1));
            const auto index_to_free = index_to_free_dist(gen);
            basic.free(basic_ptrs.at(index_to_free));
            basic_ptrs.erase(basic_ptrs.begin() + index_to_free);

            coalescing.free(coalescing_ptrs.at(index_to_free));
            coalescing_ptrs.erase(coalescing_ptrs.begin() + index_to_free);
        }
    }

    printf("LOOPS: %llu\n", loop_count);

    printf("BASIC: Fragmentation %f In use %f\n", basic.get_fragmentation(),
           static_cast<double>(basic.get_used_bytes()) / static_cast<double>(basic.original_size));
    printf("COALESCING: Fragmentation %f In use %f\n", coalescing.get_fragmentation(),
           static_cast<double>(coalescing.get_used_bytes()) / static_cast<double>(coalescing.original_size));
}
