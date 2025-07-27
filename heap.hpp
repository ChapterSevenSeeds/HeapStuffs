#pragma once

#include <cstdint>

#include "interfaces.hpp"
#include "node.hpp"

class heap final {
public:
    constexpr static size_t ALLOC_ALIGNMENT = 8;

    node *root = nullptr;
    size_t original_size = 0;
    alloc_strategy *alloc_strategy_inst;
    free_strategy *free_strategy_inst;

    explicit
    heap(const size_t size, alloc_strategy *alloc_strategy, free_strategy *free_strategy) : original_size(size),
        alloc_strategy_inst(alloc_strategy), free_strategy_inst(free_strategy) {
        if (size / ALLOC_ALIGNMENT * ALLOC_ALIGNMENT != size) {
            throw std::runtime_error("Heap size must be divisible by ALLOC_ALIGNMENT");
        }

        root = static_cast<node *>(std::malloc(size + sizeof(node)));
        root->size = size;
        root->in_use = false;
        root->has_next = false;
        root->has_previous = false;
    }

    heap(const heap &other) {
        original_size = other.original_size;
        root = static_cast<node *>(std::malloc(original_size + sizeof(node)));
        root->size = original_size;
        root->in_use = false;
        root->has_next = false;
        root->has_previous = false;

        alloc_strategy_inst = other.alloc_strategy_inst;
        free_strategy_inst = other.free_strategy_inst;
    }

    heap(heap &&other) noexcept {
        if (this == &other) {
            return;
        }

        root = other.root;
        other.root = nullptr;
        original_size = other.original_size;
        alloc_strategy_inst = other.alloc_strategy_inst;
        free_strategy_inst = other.free_strategy_inst;
    }

    ~heap() {
        std::free(root);
    }

    [[nodiscard]] std::string get_heap_type() const {
        return std::string(typeid(*alloc_strategy_inst).name()) + " " + std::string(typeid(*free_strategy_inst).name());
    }

    [[nodiscard]] void *get_end() const {
        return reinterpret_cast<uint8_t *>(root) + original_size;
    }

    [[nodiscard]] size_t get_used_bytes() const {
        auto current_block = root;
        size_t size = 0;
        while (current_block != nullptr) {
            if (current_block->in_use) {
                size += current_block->size;
            }

            current_block = current_block->get_next();
        }

        return size;
    }

    static size_t align_size(const size_t size) {
        if (size % ALLOC_ALIGNMENT == 0) {
            return size;
        }

        return size / ALLOC_ALIGNMENT * (ALLOC_ALIGNMENT + 1);
    }

    [[nodiscard]] void *alloc(const size_t size) const {
        const auto aligned_size = align_size(size);

        return alloc_strategy_inst->alloc(root, aligned_size);
    }

    void free(void *ptr) const {
        const auto node_to_free = reinterpret_cast<node *>(static_cast<uint8_t *>(ptr) - sizeof(node));

        if (node_to_free == nullptr) {
            return;
        }

        free_strategy_inst->free(node_to_free);
    }

    [[nodiscard]] double get_fragmentation() const {
        size_t max_free_size = 0;
        size_t total_free = 0;
        auto current_block = root;
        while (current_block != nullptr) {
            if (!current_block->in_use) {
                total_free += current_block->size;

                if (current_block->size > max_free_size) {
                    max_free_size = current_block->size;
                }
            }

            current_block = current_block->get_next();
        }

        return 1.0 - static_cast<double>(max_free_size) / static_cast<double>(total_free);
    }
};
