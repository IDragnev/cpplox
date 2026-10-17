#include "cpplox/runtime/Display.hpp"
#include "cpplox/runtime/List.hpp"
#include "cpplox/runtime/ObjectFormatter.hpp"
#include "cpplox/runtime/Reflect.hpp"
#include "cpplox/core/Algorithm.hpp"
#include "cpplox/core/ValueFormatter.hpp"
#include "cpplox/core/Vector.hpp"

#include <iterator>
#include <string>
#include <string_view>

namespace cpplox {
    namespace {
        void appendDisplay(std::string& out,
                           const Value& v,
                           Vector<const List*>& active);

        void appendList(std::string& out,
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

                appendDisplay(out, list.values[i], active);
            }

            out += ']';
            active.removeBack();
        }

        void appendDisplay(std::string& out,
                           const Value& v,
                           Vector<const List*>& active) {
            if (v.isString()) {
                const String& s = v.asString();
                out += '"';
                out.append(s.c_str(), s.size());
                out += '"';
            } else if (isList(v)) {
                const List* list = v.asObject()->as<List>();
                appendList(out, *list, active);
            } else if (v.isObject()) {
                fmt::format_to(std::back_inserter(out), "{}", *v.asObject());
            } else {
                fmt::format_to(std::back_inserter(out), "{}", v);
            }
        }
    } // namespace

    String display(const Value& v) {
        std::string out;
        Vector<const List*> active;
        appendDisplay(out, v, active);

        return String(std::string_view(out));
    }

    String str(const Value& v) {
        if (v.isString()) {
            return v.asString();
        }

        return display(v);
    }
} // namespace cpplox
