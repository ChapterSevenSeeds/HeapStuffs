#pragma once
#include "basic_heap.hpp"

class reuse_small_blocks_heap : public coalescing_heap {
public:
    constexpr static auto BLOCK_REUSE_THRESHOLD_BYTES = 4;

    explicit reuse_small_blocks_heap(const size_t size) : coalescing_heap(size) {
    }

    [[nodiscard]] void *alloc(const size_t size) const override {
        // Find a free node whose size matches the requested size or by BLOCK_REUSE_THRESHOLD_BYTES more bytes

        auto current_node = root;
        node *node_closest_to_requested_size = nullptr;
        while (current_node != nullptr) {
            if (!current_node->in_use && current_node->size >= size && current_node->size - size <=
                BLOCK_REUSE_THRESHOLD_BYTES && (
                    node_closest_to_requested_size == nullptr || current_node->size < node_closest_to_requested_size->
                    size)) {
                node_closest_to_requested_size = current_node;
            }

            current_node = current_node->get_next(*this);
        }

        if (node_closest_to_requested_size != nullptr) {
            node_closest_to_requested_size->in_use = true;
            return node_closest_to_requested_size->get_data_ptr();
        }

        // If we couldn't find one, just chop off a new one like normal.
        return coalescing_heap::alloc(size);
    }
};
