#pragma once
#include "node.hpp"

class alloc_strategy {
public:
    virtual ~alloc_strategy() = default;

    virtual void *alloc(node *root, size_t aligned_size) = 0;
};

class free_strategy {
public:
    virtual ~free_strategy() = default;

    virtual void free(node *to_free) = 0;
};
