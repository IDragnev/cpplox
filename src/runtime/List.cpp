#include "cpplox/runtime/List.hpp"
#include "cpplox/runtime/Reflect.hpp"
#include "cpplox/core/Algorithm.hpp"

namespace cpplox {
    namespace {
        struct Pair {
            const List* l = nullptr;
            const List* r = nullptr;
        };

        bool structurallyEqual(const List* l,
                               const List* r,
                               Vector<Pair>& inProgress) {
            if (contains(inProgress, [l, r](const Pair& p) { return p.l == l && p.r == r; })) {
                return true;
            }

            if (l->values.getCount() != r->values.getCount()) {
                return false;
            }

            const std::size_t count = l->values.getCount();
            for (std::size_t i = 0; i < count; ++i) {
                const Value& lVal = l->values[i];
                const Value& rVal = r->values[i];

                if (isList(lVal) && isList(rVal)) {
                    inProgress.insertBack({l, r});
                    const bool equal = structurallyEqual(lVal.asObject()->as<List>(),
                                                         rVal.asObject()->as<List>(),
                                                         inProgress);
                    inProgress.removeBack();
                    if (equal == false) {
                        return false;
                    }
                }
                else if (lVal != rVal) {
                    return false;
                }
            }

            return true;
        }
    } // namespace

    List::List() : List(0) {}

    List::List(std::size_t size)
        : Object(List::TYPE)
        , values(size)
    {}

    void List::trace(gc::Visitor& v) {
        forEach(values, [&v](Value& val) {
            if (val.isObject()) {
                v.visit(val.asObject());
            }
        }); 
    }

    std::size_t List::computeAllocSize() const {
        return sizeof(List) + values.getAllocSize();
    }

    bool structurallyEqual(const List* l, const List* r) {
        Vector<Pair> inProgress;
        return structurallyEqual(l, r, inProgress);
    }
} // namespace cpplox
