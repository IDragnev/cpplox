#pragma once

#include "cpplox/core/ValueStack.hpp"
#include "cpplox/core/ValueMap.hpp"
#include "cpplox/core/Vector.hpp"
#include "cpplox/core/String.hpp"

namespace cpplox {
    class Function;
    class Closure;
    class Object;
    class Upvalue;
    class Class;

    enum class InterpretResultCode {
        OK,
        COMPILE_ERROR,
        RUNTIME_ERROR,
    };

    struct InterpretResult {
        InterpretResultCode code = InterpretResultCode::OK;
        String error = "";
    };

    template <typename Op>
    concept NumberBinaryOp =
        requires(Op op, double a, double b, Value c) { c = op(a, b); };

    class VM {
        struct CallFrame {
            Closure* closure = nullptr;
            const std::uint8_t* ip = nullptr;
            std::size_t bp = 0;
        };

    public:
        VM() = default;
        ~VM();

        VM(const VM&) = delete;
        VM& operator=(const VM&) = delete;

        InterpretResult interpret(Function* func, Vector<Object*>&& objects);

    private:
        InterpretResultCode run();
        void addObjects(Vector<Object*>&& objects);
        template <typename T, typename... Args>
        T* makeObject(Args&&... args);
        static std::size_t objectSize(Object* o);
        void runGC();
        void traceGCRoots();

        template <NumberBinaryOp Op>
        bool numBinaryOp(const Op& op);

        bool invoke(const String& name, std::uint8_t argc);
        bool invokeFromClass(Class* klass, const String& method, std::uint8_t argc);
        bool callValue(Value& v, std::uint8_t argc);
        bool call(Closure* f, std::uint8_t argc);
        void printValue(const Value& v) const;
        void defineMethod(const String& name);
        bool bindMethod(Class* klass, const String& name);

        Upvalue* captureUpvalue(std::size_t offset);
        void closeUpvalues(std::size_t offset);

        template <typename... Args>
        void runtimeError(std::string_view fmtStr, Args&&... args);
        void appendCallStackInfo(struct FmtBuffer& buf);

    private:
        ValueStack stack;
        ValueMap globals;
        Vector<CallFrame> frames;
        Vector<Object*> gcObjects;
        std::uint64_t bytesAllocated = 0;
        std::uint64_t nextGC = 1024 * 1024;
        Upvalue* openUpvalues = nullptr;
        String error = "";
        String classInitKey = "init";
    };
} // namespace cpplox