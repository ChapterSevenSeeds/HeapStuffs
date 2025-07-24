#pragma once

#include <cstdlib>
#include <cstdint>

class basic_heap {
public:
    class node {
    public:
        node *previous = nullptr;
        node *next = nullptr;
        size_t size = 0;
        bool in_use = false;
        uint8_t data = 0;

        void *get_data_ptr() {
            return &data;
        }
    };

    node *root = nullptr;
    size_t original_size = 0;

    explicit basic_heap(const size_t size) {
        root = static_cast<node *>(std::malloc(size + sizeof(node)));
        root->previous = nullptr;
        root->next = nullptr;
        root->size = size;
        root->in_use = false;
        original_size = size;
    }

    [[nodiscard]] size_t get_used_bytes() const {
        auto current_block = root;
        size_t size = 0;
        while (current_block != nullptr) {
            if (current_block->in_use) {
                size += current_block->size;
            }

            current_block = current_block->next;
        }

        return size;
    }

    virtual ~basic_heap() {
        std::free(root);
    }

    [[nodiscard]] void *alloc(const size_t size) const {
        // Find the first free block that is at least as big as the request.
        auto current_block = root;
        while (current_block != nullptr && (current_block->size < size || current_block->in_use)) {
            current_block = current_block->next;
        }

        if (current_block == nullptr) {
            return nullptr;
        }

        // If the block size minus the requested size is at least 8 bytes larger than the size of a node, chop off the extra into a new node.
        if (current_block->size - size >= sizeof(node) + 8) {
            auto *current_block_data = current_block->get_data_ptr();
            const auto new_free_node = static_cast<node *>(current_block_data + size);

            // The new free node's size is the difference between the two nodes minus the node struct overhead.
            new_free_node->size = current_block->size - size - sizeof(node);
            new_free_node->in_use = false;

            current_block->size = size;
            current_block->in_use = true;

            new_free_node->next = current_block->next;
            current_block->next = new_free_node;
            new_free_node->previous = current_block;

            if (new_free_node->next != nullptr) {
                new_free_node->next->previous = new_free_node;
            }

            return current_block->get_data_ptr();
        }

        current_block->in_use = true;
        return current_block->get_data_ptr();
    }

    virtual void free(void *ptr) {
        const auto node_to_free = reinterpret_cast<node *>(static_cast<uint8_t *>(ptr) - offsetof(node, data));

        if (node_to_free == nullptr) {
            return;
        }

        node_to_free->in_use = false;
    }

    [[nodiscard]] double get_fragmentation() const {
        size_t max_free_size = 0;
        auto current_block = root;
        while (current_block != nullptr) {
            if (!current_block->in_use && current_block->size > max_free_size) {
                max_free_size = current_block->size;
            }

            current_block = current_block->next;
        }

        return 1.0 - static_cast<double>(max_free_size) / static_cast<double>(original_size);
    }
};
