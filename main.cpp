#include <algorithm>
#include <cstdio>
#include <list>
#include <random>
#include <iostream>

#include "alloc_strategies.hpp"
#include "heap.hpp"
#include "free_strategies.hpp"

constexpr auto SIZE = 8192 * 128;

struct alloc_group {
    void *ptr;
    size_t size;

    alloc_group(void *ptr, const size_t size) : ptr(ptr), size(size) {
    }
};

struct loop_group {
    std::vector<alloc_group> allocs{};
    heap heap_inst;

    template<typename... Args>
    explicit loop_group(Args... args) : heap_inst(std::forward<Args>(args)...) {
    }

    [[nodiscard]] double get_efficiency() const {
        const auto total_bytes_requested = std::ranges::fold_left(allocs, static_cast<size_t>(0),
                                                                  [](const auto total, const auto current) {
                                                                      return total + current.size;
                                                                  });
        return static_cast<double>(total_bytes_requested) / static_cast<double>(heap_inst.get_used_bytes());
    }
};


size_t run(std::vector<loop_group> &loop_groups) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> byte_dist(1, static_cast<int>(std::sqrt(SIZE)));
    std::uniform_int_distribution<> should_free_dist(1, 5);
    size_t loop_count = 0;
    while (true) {
        loop_count++;

        const auto bytes_to_allocate = byte_dist(gen);
        const auto should_free = should_free_dist(gen) > 1;

        for (auto &h: loop_groups) {
            const auto ptr = h.heap_inst.alloc(bytes_to_allocate);

            if (ptr == nullptr) {
                std::cout << "Failed allocation on " << h.heap_inst.get_heap_type() << std::endl;
                return loop_count;
            }

            h.allocs.emplace_back(ptr, bytes_to_allocate);

            if (should_free && !h.allocs.empty()) {
                std::uniform_int_distribution<> index_to_free_dist(0, static_cast<int>(h.allocs.size() - 1));
                const auto index_to_free = index_to_free_dist(gen);
                h.heap_inst.free(h.allocs.at(index_to_free).ptr);
                h.allocs.erase(h.allocs.begin() + index_to_free);
            }
        }
    }
}

int main() {
    first_fit_split_anything_alloc_strategy first_fit_split_anything_alloc_strategy_inst{};
    best_fit_split_anything_alloc_strategy best_fit_split_anything_alloc_strategy_inst{};
    first_fit_split_25_alloc_strategy first_fit_split_25_alloc_strategy_inst{};
    best_fit_split_25_alloc_strategy best_fit_split_25_alloc_strategy_inst{};
    first_fit_split_power_of_two_alloc_strategy first_fit_split_power_of_two_alloc_strategy_inst{};
    best_fit_split_power_of_two_alloc_strategy best_fit_split_power_of_two_alloc_strategy{};
    simple_free_strategy simple_free_strategy_inst{};
    coalesce_free_blocks_free_strategy coalesce_free_blocks_free_strategy_inst{};

    std::vector loop_groups = {
        loop_group{SIZE, &first_fit_split_anything_alloc_strategy_inst, &simple_free_strategy_inst},
        loop_group{SIZE, &first_fit_split_anything_alloc_strategy_inst, &coalesce_free_blocks_free_strategy_inst},
        loop_group{SIZE, &best_fit_split_anything_alloc_strategy_inst, &simple_free_strategy_inst},
        loop_group{SIZE, &best_fit_split_anything_alloc_strategy_inst, &coalesce_free_blocks_free_strategy_inst},
        loop_group{SIZE, &first_fit_split_25_alloc_strategy_inst, &simple_free_strategy_inst},
        loop_group{SIZE, &first_fit_split_25_alloc_strategy_inst, &coalesce_free_blocks_free_strategy_inst},
        loop_group{SIZE, &best_fit_split_25_alloc_strategy_inst, &simple_free_strategy_inst},
        loop_group{SIZE, &best_fit_split_25_alloc_strategy_inst, &coalesce_free_blocks_free_strategy_inst},
        loop_group{SIZE, &first_fit_split_power_of_two_alloc_strategy_inst, &simple_free_strategy_inst},
        loop_group{SIZE, &first_fit_split_power_of_two_alloc_strategy_inst, &coalesce_free_blocks_free_strategy_inst},
        loop_group{SIZE, &best_fit_split_power_of_two_alloc_strategy, &simple_free_strategy_inst},
        loop_group{SIZE, &best_fit_split_power_of_two_alloc_strategy, &coalesce_free_blocks_free_strategy_inst}
    };

    const auto loop_count = run(loop_groups);

    printf("LOOPS: %zu\n", loop_count);

    for (auto &h: loop_groups) {
        printf("**** %s\n\tFragmentation %-10f In use %-10f Efficiency %-10f\n", h.heap_inst.get_heap_type().c_str(),
               h.heap_inst.get_fragmentation(),
               static_cast<double>(h.heap_inst.get_used_bytes()) / static_cast<double>(h.heap_inst.original_size),
               h.get_efficiency());
    }
}
