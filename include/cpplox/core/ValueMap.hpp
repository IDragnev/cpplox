#pragma once

#include "cpplox/core/String.hpp"
#include "cpplox/core/Value.hpp"
#include "cpplox/core/Vector.hpp"

namespace cpplox {
    class ValueMap {
        struct Entry {
            String key;
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

        void insert(String key, Value v);
        bool remove(const String& key, Value& v);
        bool find(const String& key, Value& v) const;
        bool contains(const String& key) const;

        bool isEmpty() const;
        void clear();

        void swap(ValueMap& other);

        template <typename F>
        void forEachValue(const F& f);

    private:
        ValueMap(std::size_t tableSize);

        std::size_t findSlot(const String& key) const;
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
    void ValueMap::forEachValue(const F& f) {
        const std::size_t size = table.getCount();
        for (std::size_t i = 0; i < size; ++i) {
            Entry& e = table[i];
            if (e.isEmpty == false) {
                f(e.value);
            }
        }
    }
} // namespace cpplox