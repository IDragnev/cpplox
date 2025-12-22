#pragma once

#include <type_traits>

namespace cpplox {
    template <typename T>
    class Vector {
    private:
        static_assert(std::is_default_constructible_v<T>,
                      "Vector<T> requires T to be default constructible");
        static_assert(std::is_copy_assignable_v<T>,
                      "Vector<T> requires T to be copy assignable");

    public:
        Vector() = default;
        explicit Vector(std::size_t count);
        Vector(Vector&& source) noexcept;
        Vector(const Vector& source);
        ~Vector();

        Vector& operator=(Vector&& rhs) noexcept;
        Vector& operator=(const Vector& rhs);

    public:
        void insertBack(T&& item);
        void insertBack(const T& item);
        void insertAt(std::size_t position, T&& item);
        void insertAt(std::size_t position, const T& item);
        void removeAt(std::size_t position);
        void removeBack();

        T& operator[](std::size_t position);
        const T& operator[](std::size_t position) const;

        const T& back() const;
        T& back();

        const T* data() const;
        T* data();

        bool isEmpty() const noexcept;
        void clear() noexcept;
        void reserve(std::size_t size);

        void swap(Vector<T>& other) noexcept;

        std::size_t getSize() const noexcept;
        std::size_t getCount() const noexcept;

    private:
        void growIfFull();

        template <typename F>
        void doInsertBack(F&& item);
        template <typename F>
        void doInsertAt(std::size_t position, F&& item);

        void nullifyMembers() noexcept;

    private:
        std::size_t size = 0;
        std::size_t count = 0;
        T* items = nullptr;
    };

    template <typename T>
    bool operator==(const Vector<T>& lhs, const Vector<T>& rhs);
    template <typename T>
    inline bool operator!=(const Vector<T>& lhs, const Vector<T>& rhs) {
        return !(lhs == rhs);
    }

    template <typename T>
    inline void swap(Vector<T>& lhs, Vector<T>& rhs) noexcept {
        lhs.swap(rhs);
    }
} // namespace cpplox

#include "VectorImpl.hpp"