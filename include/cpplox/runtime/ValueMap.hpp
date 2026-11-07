#pragma once

#include "cpplox/core/Value.hpp"
#include "cpplox/core/Vector.hpp"

namespace cpplox {
    class RuntimeStr;

    class ValueMap {
        struct Entry {
            RuntimeStr* key = nullptr;
            Value value;
            bool isEmpty = true;
        };
    public:
        ValueMap();
        ValueMap(ValueMap&& src) noexcept;
        ValueMap(const ValueMap& src);
        ~ValueMap() = default;

        ValueMap& operator=(ValueMap&& src) noexcept;
        ValueMap& operator=(const ValueMap& src);

        void insert(RuntimeStr* key, Value v);
        bool remove(const RuntimeStr* key, Value& v);
        bool find(const RuntimeStr* key, Value& v) const;
        bool contains(const RuntimeStr* key) const;

        bool isEmpty() const;
        void clear();
        
        std::size_t getAllocSize() const { return table.getAllocSize(); }

        void swap(ValueMap& other);

        template <typename F>
        void forEach(const F& f);

    private:
        ValueMap(std::size_t tableSize);

        std::size_t findSlot(const RuntimeStr* key) const;
        void growIfFillingUp();

        static void insertEntries(const Vector<Entry>& table, ValueMap& map);

    private:
        Vector<Entry> table;
        std::size_t count = 0;
    };

    inline void swap(ValueMap& a, ValueMap& b) {
        a.swap(b);
    }

    template <typename F>
    void ValueMap::forEach(const F& f) {
        const std::size_t size = table.getCount();
        for (std::size_t i = 0; i < size; ++i) {
            Entry& e = table[i];
            if (e.isEmpty == false) {
                f(e.key, e.value);
            }
        }
    }
} // namespace cpplox
