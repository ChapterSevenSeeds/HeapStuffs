#pragma once
#include "interfaces.hpp"
#include "node.hpp"

// Finds the first free node that can fit the requested size.
inline node *find_first_fit(node *root, const size_t size) {
    auto current_block = root;
    while (current_block != nullptr && (current_block->size < size || current_block->in_use)) {
        current_block = current_block->get_next();
    }

    return current_block;
}

// Finds the smallest node that can fit the requested size.
inline node *find_best_fit(node *root, const size_t size) {
    node *best_fit = nullptr;
    for (auto current_block = root; current_block != nullptr; current_block = current_block->get_next()) {
        if (current_block->in_use) {
            continue;
        }

        if (current_block->size >= size && (best_fit == nullptr || current_block->size < best_fit->size)) {
            best_fit = current_block;
        }
    }

    return best_fit;
}

/*!
 * Splits the node if it can fit the requested size plus another node header.
 * @return A pointer to the data area of the first argument.
 */
inline void *return_with_split_if_possible(node *n, const size_t size) {
    if (n->size - size >= sizeof(node)) {
        n->split(size);
        return n->get_data_ptr();
    }

    n->in_use = true;
    return n->get_data_ptr();
}

/*!
 * Splits the node if it can fit the requested size plus another node header PLUS a percentage of the requested bytes.
 * @return A pointer to the data area of the first argument.
 */
inline void *return_with_split_with_threshold(node *n, const size_t size, const double free_space_threshold) {
    if (n->size - size >= sizeof(node) + static_cast<int>(free_space_threshold * static_cast<double>(size))) {
        n->split(size);
        return n->get_data_ptr();
    }

    n->in_use = true;
    return n->get_data_ptr();
}

class first_fit_split_anything_alloc_strategy final : public alloc_strategy {
public:
    void *alloc(node *root, const size_t aligned_size) override {
        const auto first_fit = find_first_fit(root, aligned_size);

        if (first_fit == nullptr) {
            return nullptr;
        }

        return return_with_split_if_possible(first_fit, aligned_size);
    }
};

class best_fit_split_anything_alloc_strategy final : public alloc_strategy {
public:
    void *alloc(node *root, const size_t aligned_size) override {
        const auto best_fit = find_best_fit(root, aligned_size);

        if (best_fit == nullptr) {
            return nullptr;
        }

        return return_with_split_if_possible(best_fit, aligned_size);
    }
};

class first_fit_split_25_alloc_strategy final : public alloc_strategy {
public:
    void *alloc(node *root, const size_t aligned_size) override {
        const auto first_fit = find_first_fit(root, aligned_size);

        if (first_fit == nullptr) {
            return nullptr;
        }

        return return_with_split_with_threshold(first_fit, aligned_size, 0.25);
    }
};

class best_fit_split_25_alloc_strategy final : public alloc_strategy {
public:
    void *alloc(node *root, const size_t aligned_size) override {
        const auto best_fit = find_best_fit(root, aligned_size);

        if (best_fit == nullptr) {
            return nullptr;
        }

        return return_with_split_with_threshold(best_fit, aligned_size, 0.25);
    }
};

class best_fit_split_power_of_two_alloc_strategy final : public alloc_strategy {
public:
    void *alloc(node *root, const size_t aligned_size) override {
        const auto resized_log = log2(static_cast<double>(aligned_size));
        const auto power_of_2_size = static_cast<size_t>(pow(2, ceil(resized_log)));

        const auto best_fit = find_best_fit(root, power_of_2_size);

        if (best_fit == nullptr) {
            return nullptr;
        }

        return return_with_split_if_possible(best_fit, power_of_2_size);
    }
};
