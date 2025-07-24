#pragma once
#include "basic_heap.hpp"

class coalescing_heap final : public basic_heap {
public:
    explicit coalescing_heap(const size_t size) : basic_heap(size) {
    }

    void free(void *ptr) override {
        const auto node_to_free = reinterpret_cast<node *>(static_cast<uint8_t *>(ptr) - offsetof(node, data));

        if (node_to_free == nullptr) {
            return;
        }

        node_to_free->in_use = false;

        if (node_to_free->next != nullptr && !node_to_free->next->in_use) {
            node_to_free->size += node_to_free->next->size + sizeof(node);
            node_to_free->next = node_to_free->next->next;
            if (node_to_free->next != nullptr) {
                node_to_free->next->previous = node_to_free;
            }
        }

        if (node_to_free->previous != nullptr && !node_to_free->previous->in_use) {
            node_to_free->previous->size += node_to_free->size + sizeof(node);
            node_to_free->previous->next = node_to_free->next;
            if (node_to_free->next != nullptr) {
                node_to_free->next->previous = node_to_free->previous;
            }
        }
    }
};
