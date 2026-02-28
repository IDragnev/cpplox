#pragma once

#include "cpplox/core/Vector.hpp"

#include <concepts>
#include <utility>

namespace cpplox {
    template <typename T, typename F>
        requires std::invocable<F, const T&>
    void forEach(const Vector<T>& v, const F& f) {
        const auto size = v.getCount();
        for (std::size_t i = 0; i < size; ++i) {
            f(v[i]);
        }
    }

    template <typename T, typename F>
        requires std::invocable<F, T&> ||
                 std::invocable<F, const T&>
    void forEach(Vector<T>& v, const F& f) {
        const auto size = v.getCount();
        for (std::size_t i = 0; i < size; ++i) {
            f(v[i]);
        }
    }

    template <typename T, typename Pred>
        requires std::predicate<Pred, const T&>
    void removeIf(Vector<T>& vec, Pred pred) {
        const std::size_t n = vec.getCount();
        std::size_t write = 0;

        for (std::size_t read = 0; read < n; ++read) {
            if (!pred(vec[read])) {
                if (write != read) {
                    vec[write] = std::move(vec[read]);
                }
                ++write;
            }
        }

        vec.removeLastN(n - write);
    }
} // namespace cpplox