#pragma once
#include <cstdint>

class node {
public:
    size_t size = 0;
    size_t previous_size = 0;
    bool in_use: 1 = false;
    bool has_next: 1 = false;
    bool has_previous: 1 = false;

    void *get_data_ptr() {
        return reinterpret_cast<uint8_t *>(this) + sizeof(node);
    }

    node *get_next() {
        if (!has_next) {
            return nullptr;
        }

        return reinterpret_cast<node *>(static_cast<uint8_t *>(get_data_ptr()) + size);
    }

    node *get_previous() {
        if (!has_previous) {
            return nullptr;
        }

        return reinterpret_cast<node *>(reinterpret_cast<uint8_t *>(this) - previous_size - sizeof(node));
    }

    /*!
     * Shrinks `this` to a size of `size` and splits off the extra into a new free node.
     * @param size The size to shrink the current node down to.
     */
    void split(const size_t size) {
        if (this->size - size < sizeof(node)) {
            throw std::invalid_argument("A node's allocated space must have room for a new node to be split.");
        }

        auto *current_block_data = this->get_data_ptr();
        const auto new_free_node = reinterpret_cast<node *>(static_cast<uint8_t *>(current_block_data) + size);

        new_free_node->size = this->size - size - sizeof(node);
        new_free_node->in_use = false;
        new_free_node->has_next = this->has_next;
        if (new_free_node->has_next) {
            const auto node_after_new_free_node = new_free_node->get_next();
            node_after_new_free_node->has_previous = true;
            node_after_new_free_node->previous_size = new_free_node->size;
        }

        this->size = size;
        this->in_use = true;
        this->has_next = true;
        new_free_node->has_previous = true;
        new_free_node->previous_size = this->size;
    }
};
