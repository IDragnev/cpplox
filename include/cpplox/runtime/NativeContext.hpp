#pragma once

#include "cpplox/core/Value.hpp"

#include <fmt/base.h>

#include <cstddef>
#include <string_view>
#include <utility>

namespace cpplox {
    class Heap;
    class Object;
    class List;
    class RuntimeStr;
    class String;

    class NativeContext {
    public:
        explicit NativeContext(Heap& heap) : heap(&heap) {}

        void resized(Object* obj);
        // Never collects; returns nullptr when out of memory.
        List* newList(std::size_t count);
        RuntimeStr* newString(String str);

        template <typename... Args>
        Value error(fmt::format_string<Args...> fmtStr, Args&&... args) {
            const auto r = fmt::format_to_n(
                message, sizeof(message), fmtStr, std::forward<Args>(args)...);
            messageLength = r.size < sizeof(message) ? r.size : sizeof(message);
            failed = true;

            return Value::nil();
        }
        Value outOfMemory() { return error("Out of memory."); }

        bool hasFailed() const { return failed; }
        std::string_view getMessage() const { return {message, messageLength}; }

    private:
        template <typename T, typename... Args>
        T* make(Args&&... args);

    private:
        Heap* heap = nullptr;
        bool failed = false;
        std::size_t messageLength = 0;
        char message[256];
    };
} // namespace cpplox
