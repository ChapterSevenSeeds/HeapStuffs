#pragma once
#include "interfaces.hpp"

class simple_free_strategy final : public free_strategy {
public:
    void free(node *to_free) override {
        to_free->in_use = false;
    }
};

class coalesce_free_blocks_free_strategy final : public free_strategy {
public:
    void free(node *to_free) override {
        to_free->in_use = false;

        for (const auto n: {to_free, to_free->get_previous()}) {
            if (n == nullptr || n->in_use || !n->has_next || n->get_next()->in_use) {
                continue;
            }

            const auto node_after_node_to_free = n->get_next();
            n->size += node_after_node_to_free->size + sizeof(node);
            n->has_next = node_after_node_to_free->has_next;

            if (n->has_next) {
                const auto node_after_coalesced_nodes = n->get_next();
                node_after_coalesced_nodes->has_previous = true;
                node_after_coalesced_nodes->previous_size = n->size;
            }
        }
    }
};
