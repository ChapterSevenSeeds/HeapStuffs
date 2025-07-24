#pragma once

#include <cstdlib>
#include <cstdint>

class basic_heap {
public:
    constexpr static auto MAGIC = 1234567890;

    class node {
    public:
        uint32_t magic = MAGIC;
        bool in_use = false;
        size_t size = 0;

        void *get_data_ptr() {
            return reinterpret_cast<uint8_t *>(this) + sizeof(node);
        }

        node *get_next(const basic_heap &heap) {
            const auto n = reinterpret_cast<node *>(static_cast<uint8_t*>(get_data_ptr()) + size);
            if (n > heap.get_end()) return nullptr;
            return n;
        }

        [[nodiscard]] node *get_previous(const basic_heap &heap) const {
            auto n = heap.root;
            while (n != nullptr && n->get_next(heap) != this) {
                n = n->get_next(heap);
            }

            return n;
        }
    };

    node *root = nullptr;
    size_t original_size = 0;

    [[nodiscard]] void *get_end() const {
        return reinterpret_cast<uint8_t *>(root) + original_size;
    }

    explicit basic_heap(const size_t size) {
        root = static_cast<node *>(std::malloc(size + sizeof(node)));
        root->size = size;
        root->in_use = false;
        root->magic = MAGIC;
        original_size = size;
    }

    [[nodiscard]] size_t get_used_bytes() const {
        auto current_block = root;
        size_t size = 0;
        while (current_block != nullptr) {
            if (current_block->in_use) {
                size += current_block->size;
            }

            current_block = current_block->get_next(*this);
        }

        return size;
    }

    virtual ~basic_heap() {
        std::free(root);
    }

    [[nodiscard]] virtual void *alloc(const size_t size) const {
        // Find the first free block that is at least as big as the request.
        auto current_block = root;
        while (current_block != nullptr && (current_block->size < size || current_block->in_use)) {
            current_block = current_block->get_next(*this);
        }

        if (current_block == nullptr) {
            return nullptr;
        }

        // If the block size minus the requested size is at least 8 bytes larger than the size of a node, chop off the extra into a new node.
        if (current_block->size - size >= sizeof(node) + 8) {
            auto *current_block_data = current_block->get_data_ptr();
            const auto new_free_node = reinterpret_cast<node *>(static_cast<uint8_t *>(current_block_data) + size);

            // The new free node's size is the difference between the two nodes minus the node struct overhead.
            new_free_node->size = current_block->size - size - sizeof(node);
            new_free_node->in_use = false;
            new_free_node->magic = MAGIC;

            current_block->size = size;
            current_block->in_use = true;

            return current_block->get_data_ptr();
        }

        current_block->in_use = true;
        return current_block->get_data_ptr();
    }

    virtual void free(void *ptr) {
        const auto node_to_free = reinterpret_cast<node *>(static_cast<uint8_t *>(ptr) - sizeof(node));

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

            current_block = current_block->get_next(*this);
        }

        return 1.0 - static_cast<double>(max_free_size) / static_cast<double>(original_size);
    }
};
