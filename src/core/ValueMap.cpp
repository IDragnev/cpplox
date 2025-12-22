#include "cpplox/core/ValueMap.hpp"

#include <cmath>

namespace cpplox {
    static inline const std::size_t DEFAULT_SIZE = 16;
    static inline const std::size_t GROWTH_FACTOR = 2;
    static inline const double MAX_LOAD_FACTOR = 0.75;

    ValueMap::ValueMap() : ValueMap(DEFAULT_SIZE) {}

    ValueMap::ValueMap(std::size_t tableSize) : table(tableSize) {}

    ValueMap::ValueMap(const ValueMap& src)
        : ValueMap(src.table.getCount())
    {
        count = src.count;

        std::size_t tableSize = table.getCount();
        for (std::size_t slot = 0; slot < tableSize; ++slot) {
            const Entry& e = src.table[slot];
            if (e.isEmpty == false) {
                table[slot] = src.table[slot];
            }
        }
    }

    ValueMap::ValueMap(ValueMap&& src) noexcept
        : table(std::move(src.table))
        , count(src.count)
    {
        src.table = Vector<Entry>(DEFAULT_SIZE);
        src.count = 0;
    }

    ValueMap& ValueMap::operator=(ValueMap&& src) noexcept {
        if (this != &src) {
            ValueMap temp(std::move(src));
            swap(temp);
        }

        return *this;
    }

    ValueMap& ValueMap::operator=(const ValueMap& src) {
        if (this != &src) {
            ValueMap temp(src);
            swap(temp);
        }

        return *this;
    }

    void ValueMap::insert(String key, Value v) {
        growIfFillingUp();

        std::size_t slot = findSlot(key);
        Entry& entry = table[slot];
        entry.key = std::move(key);
        entry.value = std::move(v);

        if (entry.isEmpty) {
            entry.isEmpty = false;
            ++count;
        }
    }

    bool ValueMap::find(const String& key, Value& v) const {
        if (isEmpty()) {
            return false;
        }

        std::size_t slot = findSlot(key);
        const Entry& e = table[slot];
        if (e.isEmpty == false) {
            v = e.value;
            return true;
        } else {
            return false;
        }
    }

    bool ValueMap::contains(const String& key) const {
        if (isEmpty()) {
            return false;
        }

        std::size_t slot = findSlot(key);
        return table[slot].isEmpty == false;
    }

    bool ValueMap::isEmpty() const {
        return count == 0;
    }

    void ValueMap::clear() {
        *this = ValueMap();
    }

    bool ValueMap::remove(const String& key, Value& v) {
        // todo
        (void)key;
        (void)v;
        return false;
    }

    std::size_t ValueMap::findSlot(const String& key) const {
        std::size_t tableSize = table.getCount();
        std::size_t slot = key.hashValue() % tableSize;

        for (;;) {
            const Entry& entry = table[slot];
            if (entry.isEmpty || entry.key == key) {
                return slot;
            }

            slot = (slot + 1) % tableSize;
        }
    }

    void ValueMap::growIfFillingUp() {
        const auto max = static_cast<std::size_t>(
            std::floor(table.getCount() * MAX_LOAD_FACTOR));
        if (count + 1 > max) {
            std::size_t newSize = table.getCount() * GROWTH_FACTOR;
            ValueMap temp(newSize);
            insertEntries(table, temp);
            swap(temp);
        }
    }

    void ValueMap::insertEntries(const Vector<Entry>& table, ValueMap& map) {
        std::size_t tableSize = table.getCount();
        for (std::size_t slot = 0; slot < tableSize; ++slot) {
            const Entry& e = table[slot];
            if (e.isEmpty == false) {
                map.insert(e.key, e.value);
            }
        }
    }

    void ValueMap::swap(ValueMap& other) {
        using std::swap;
        swap(table, other.table);
        swap(count, other.count);
    }
}