#pragma once

namespace cpplox {
    class Object;

    namespace gc {
        class Visitor {
        public:
            virtual ~Visitor() = default;
            virtual void visit(Object*) = 0;
        };
    } // namespace gc
} // namespace cpplox