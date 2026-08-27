#pragma once

#include "cpplox/runtime/GCVisitor.hpp"

#include <cstddef>

namespace cpplox {
    enum class ObjectType {
        FUNCTION,
        CLOSURE,
        UPVALUE,
        CLASS,
        INSTANCE,
        BOUND_METHOD,
        NATIVE_FUNCTION,
    };

    template <typename T>
    concept HasTypeTag = requires { T::TYPE; };

    class Object {
    public:
        Object(ObjectType t, std::size_t size) : _type(t), _size(size) {}
        virtual ~Object() = default;

        virtual void trace(gc::Visitor& v) = 0;

        ObjectType type() const { return _type; }
        bool hasType(ObjectType t) const { return _type == t; }
        std::size_t size() const { return _size; }

        template <HasTypeTag T>
        T* as() {
            return _type == T::TYPE ? static_cast<T*>(this) : nullptr;
        }
        template <HasTypeTag T>
        const T* as() const {
            return _type == T::TYPE ? static_cast<const T*>(this) : nullptr;
        }

    public:
        bool isReachable = false;
        Object* nextObject = nullptr;

    private:
        ObjectType _type;
        std::size_t _size;
    };
} // namespace cpplox
