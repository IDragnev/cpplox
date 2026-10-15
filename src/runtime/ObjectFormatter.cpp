#include "cpplox/runtime/ObjectFormatter.hpp"
#include "cpplox/runtime/Reflect.hpp"
#include "cpplox/core/ValueFormatter.hpp"
#include "cpplox/core/Algorithm.hpp"

#include <iterator>

namespace cpplox::detail {
    void formatList(std::string& out,
                    const List& list,
                    Vector<const List*>& active) {
        if (contains(active, [&list](const List* l) { return l == &list; })) {
            out += "[...]";
            return;
        }

        active.insertBack(&list);
        out += '[';

        for (std::size_t i = 0; i < list.values.getCount(); ++i) {
            if (i > 0) {
                out += ", ";
            }

            const Value& v = list.values[i];
            if (v.isObject() == false) {
                fmt::format_to(std::back_inserter(out), "{:?}", v);
            } else if (isList(v)) {
                const List* inner = v.asObject()->as<List>();
                formatList(out, *inner, active);
            } else {
                fmt::format_to(std::back_inserter(out), "{}", *v.asObject());
            }
        }

        out += ']';
        active.removeBack();
    }
} // namespace cpplox::detail
