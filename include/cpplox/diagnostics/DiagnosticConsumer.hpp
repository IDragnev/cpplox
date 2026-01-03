#pragma once

namespace cpplox {
    struct Diagnostic;

    class DiagnosticConsumer {
    public:
        virtual ~DiagnosticConsumer() = default;
        virtual void consume(Diagnostic&& d) = 0;
    };
} // namespace cpplox