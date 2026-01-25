#pragma once

namespace cpplox {
    enum class ObjectType {
        FUNCTION,
        CLOSURE,
    };

    template <typename T>
    concept HasTypeTag = requires { T::TYPE; };

    class Object {
    public:
        explicit Object(ObjectType t) : _type(t) {}
        virtual ~Object() = default;

        ObjectType type() const { return _type; }
        bool hasType(ObjectType t) const { return _type == t; }

        template <HasTypeTag T>
        T* as() {
            return _type == T::TYPE ? static_cast<T*>(this) : nullptr;
        }
        template <HasTypeTag T>
        const T* as() const {
            return _type == T::TYPE ? static_cast<const T*>(this) : nullptr;
        }

    private:
        ObjectType _type;
    };
} // namespace cpplox
