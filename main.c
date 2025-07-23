#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct basic_heap_node {
    struct basic_heap_node *previous;
    struct basic_heap_node *next;
    // The size of bytes available to the chunk. I.E. if the user requested 30 bytes, this will be 30.
    size_t size;
    uint8_t data;
};

struct basic_heap {
    struct basic_heap_node *free;
    size_t original_size;
};

void init_basic_heap(struct basic_heap *heap) {
    heap->free = nullptr;
    heap->original_size = 0;
}

void init_basic_node(struct basic_heap_node *node) {
    node->previous = nullptr;
    node->next = nullptr;
}

void *get_basic_heap_node_data(const struct basic_heap_node *node) {
    return (void *) &node->data;
}

void make_basic_heap(struct basic_heap *heap, const size_t size) {
    heap->free = (struct basic_heap_node *) malloc(size + sizeof(struct basic_heap_node));
    init_basic_node(heap->free);
    heap->free->size = size;
    heap->original_size = size;
}

void *basic_heap_alloc(struct basic_heap *heap, const size_t size) {
    // Find the first free block that is at least as big as the request.
    struct basic_heap_node *current_free = heap->free;
    while (current_free != nullptr && current_free->size < size) {
        current_free = current_free->next;
    }

    if (current_free == nullptr) {
        return nullptr;
    }

    // If the block size minus the requested size is at least 8 bytes larger than the size of a node, chop off the extra into a new node.
    if (current_free->size - size >= sizeof(struct basic_heap_node) + 8) {
        uint8_t *current_free_data = get_basic_heap_node_data(current_free);
        auto new_free_node = (struct basic_heap_node *) (current_free_data + size);

        // The new free node's size is the difference between the two nodes minus the node struct overhead.
        new_free_node->size = current_free->size - size - sizeof(struct basic_heap_node);

        // Detach the node we are going to hand back to the caller from the free list and instead insert the new free node in its place.
        new_free_node->previous = current_free->previous;
        if (current_free->previous != nullptr) {
            current_free->previous->next = new_free_node;
        }
        new_free_node->next = current_free->next;
        if (current_free->next != nullptr) {
            current_free->next->previous = new_free_node;
        }

        current_free->size = size;

        // Do we need to update the heap's free list pointer to the new free node?
        if (heap->free == current_free) {
            heap->free = new_free_node;
        }

        return get_basic_heap_node_data(current_free);
    }

    if (current_free->previous != nullptr) {
        current_free->previous->next = current_free->next;
    }
    if (current_free->next != nullptr) {
        current_free->next->previous = current_free->previous;
    }

    return get_basic_heap_node_data(current_free);
}

void basic_heap_free(struct basic_heap *heap, void *ptr) {
    auto node_to_free = (struct basic_heap_node *) (((uint8_t *) ptr) - offsetof(struct basic_heap_node, data));

    if (node_to_free == nullptr) {
        return;
    }

    // Find the end of the free list and just append us onto it.
    auto current_free = heap->free;
    while (current_free->next != nullptr) {
        current_free = current_free->next;
    }

    current_free->next = node_to_free;
    node_to_free->previous = current_free;
}

double get_basic_heap_fragmentation(const struct basic_heap *heap) {
    size_t max_free_size = 0;
    auto current_free = heap->free;
    while (current_free != nullptr) {
        if (current_free->size > max_free_size) {
            max_free_size = current_free->size;
        }

        current_free = current_free->next;
    }

    return 1.0 - ((double) max_free_size / (double) heap->original_size);
}

int main(void) {
    struct basic_heap heap;
    init_basic_heap(&heap);
    make_basic_heap(&heap, 8192);
    basic_heap_alloc(&heap, 128);
    basic_heap_alloc(&heap, 128);
    basic_heap_alloc(&heap, 128);
    basic_heap_alloc(&heap, 128);
    basic_heap_alloc(&heap, 128);
    basic_heap_alloc(&heap, 128);

    printf("Fragmentation %f", get_basic_heap_fragmentation(&heap));
}
