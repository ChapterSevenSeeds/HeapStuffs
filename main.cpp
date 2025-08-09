#include <algorithm>
#include <cstdio>
#include <list>
#include <random>
#include <iostream>
#include <ranges>

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
    bool failed = false;
    size_t failed_on_iteration = 0;

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


void run(std::vector<loop_group> &loop_groups) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> byte_dist(1, static_cast<int>(std::sqrt(SIZE)));
    std::uniform_int_distribution<> should_free_dist(1, 5);
    size_t loop_count = 0;
    const auto is_failed_predicate = [](const auto &g) { return !g.failed; };
    while (std::ranges::count_if(loop_groups, is_failed_predicate) > 0) {
        loop_count++;

        const auto bytes_to_allocate = byte_dist(gen);
        const auto should_free = should_free_dist(gen) > 1;

        for (auto &h: std::ranges::filter_view(loop_groups, is_failed_predicate)) {
            const auto ptr = h.heap_inst.alloc(bytes_to_allocate);

            if (ptr == nullptr) {
                h.failed = true;
                h.failed_on_iteration = loop_count;
                continue;
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
        loop_group{SIZE, &best_fit_split_power_of_two_alloc_strategy, &simple_free_strategy_inst},
        loop_group{SIZE, &best_fit_split_power_of_two_alloc_strategy, &coalesce_free_blocks_free_strategy_inst}
    };

    run(loop_groups);

    for (auto &h: loop_groups) {
        printf("**** %s\n\tLoops %zu Fragmentation %-10f In use %-10f Efficiency %-10f\n",
               h.heap_inst.get_heap_type().c_str(),
               h.failed_on_iteration,
               h.heap_inst.get_fragmentation(),
               static_cast<double>(h.heap_inst.get_used_bytes()) / static_cast<double>(h.heap_inst.original_size),
               h.get_efficiency());
    }
}
