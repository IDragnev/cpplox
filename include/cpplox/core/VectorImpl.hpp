namespace cpplox {
    template <typename T>
    Vector<T>::Vector(std::size_t count)
        : size(count)
        , count(count)
        , items(count > 0 ? new T[count]{} : nullptr)
    {
    }

    template <typename T>
    Vector<T>::Vector(const Vector<T>& source)
        : Vector(source.count)
    {
        for (std::size_t i = 0; i < count; ++i) {
            items[i] = source.items[i];
        }
    }

    template <typename T>
    Vector<T>::Vector(Vector&& source) noexcept
        : size(source.size)
        , count(source.count)
        , items(source.items)
    {
        source.nullifyMembers();
    }

    template <typename T>
    void Vector<T>::nullifyMembers() noexcept {
        size = 0;
        count = 0;
        items = nullptr;
    }

    template <typename T>
    Vector<T>& Vector<T>::operator=(const Vector& rhs) {
        if (this != &rhs) {
            swapContentsWith(rhs);
        }

        return *this;
    }

    template <typename T>
    Vector<T>& Vector<T>::operator=(Vector&& rhs) noexcept {
        if (this != &rhs) {
            swapContentsWith(std::move(rhs));
        }

        return *this;
    }

    template <typename T>
    void Vector<T>::swapContentsWith(Vector temporary) noexcept {
        std::swap(size, temporary.size);
        std::swap(count, temporary.count);
        std::swap(items, temporary.items);
    }

    template <typename T>
    Vector<T>::~Vector() {
        delete[] items;
    }

    template <typename T>
    void Vector<T>::clear() noexcept {
        delete[] items;
        nullifyMembers();
    }

    template <typename T>
    inline void Vector<T>::insertBack(const T& item) {
        doInsertBack(item);
    }

    template <typename T>
    inline void Vector<T>::insertBack(T&& item) {
        doInsertBack(std::move(item));
    }

    template <typename T>
    template <typename Item>
    void Vector<T>::doInsertBack(Item&& item) {
        growIfFull();

        items[count] = std::forward<Item>(item);
        ++count;
    }

    template <typename T>
    void Vector<T>::growIfFull() {
        const std::size_t GROWTH_FACTOR = 2;
        const std::size_t DEFAULT_SIZE = 8;

        if (count == size) {
            const std::size_t newSize = size > 0 ? (GROWTH_FACTOR * size) 
                                                 : DEFAULT_SIZE;

            Vector temp(newSize);
            temp.count = count;
            for (std::size_t i = 0; i < count; ++i) {
                if constexpr (std::is_nothrow_move_assignable_v<T>) {
                    temp.items[i] = std::move(items[i]);
                } else {
                    temp.items[i] = items[i];
                }
            }

            swapContentsWith(std::move(temp));
        }
    }

    template <typename T>
    inline void Vector<T>::insertAt(std::size_t position, const T& item) {
        doInsertAt(position, item);
    }

    template <typename T>
    inline void Vector<T>::insertAt(std::size_t position, T&& item) {
        doInsertAt(position, std::move(item));
    }

    template <typename T>
    template <typename Item>
    void Vector<T>::doInsertAt(std::size_t position, Item&& item) {
        if (count > 0 && position < count) {
            growIfFull();

            for (std::size_t i = count; i > position; --i) {
                items[i] = std::move(items[i - 1]);
            }
            items[position] = std::forward<Item>(item);
            ++count;
        } else {
            insertBack(std::forward<Item>(item));
        }
    }

    template <typename T>
    void Vector<T>::removeAt(std::size_t position) {
        if (count == 0) {
            return;
        }
        if (position >= count) {
            return;
        }

        if (position < count - 1) {
            const std::size_t end = count - 1;
            for (std::size_t i = position; i < end; ++i) {
                items[i] = std::move(items[i + 1]);
            }
        }

        --count;
    }

    template <typename T>
    inline T& Vector<T>::operator[](std::size_t position) {
        return items[position];
    }

    template <typename T>
    inline const T& Vector<T>::operator[](std::size_t position) const {
        return items[position];
    }

    template <typename T>
    inline bool Vector<T>::isEmpty() const noexcept {
        return count == 0;
    }

    template <typename T>
    inline std::size_t Vector<T>::getCount() const noexcept {
        return count;
    }

    template <typename T>
    inline std::size_t Vector<T>::getSize() const noexcept {
        return size;
    }
} // namespace cpplox