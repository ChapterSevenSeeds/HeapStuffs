#pragma once
#include "basic_heap.hpp"

class coalescing_heap : public basic_heap {
public:
    explicit coalescing_heap(const size_t size) : basic_heap(size) {
    }

    void free(void *ptr) override {
        const auto node_to_free = reinterpret_cast<node *>(static_cast<uint8_t *>(ptr) - sizeof(node));

        if (node_to_free == nullptr) {
            return;
        }

        node_to_free->in_use = false;

        if (node_to_free->get_next(*this) != nullptr && !node_to_free->get_next(*this)->in_use) {
            node_to_free->size += node_to_free->get_next(*this)->size + sizeof(node);
        }

        if (node_to_free->get_previous(*this) != nullptr && !node_to_free->get_previous(*this)->in_use) {
            node_to_free->get_previous(*this)->size += node_to_free->size + sizeof(node);
        }
    }
};
