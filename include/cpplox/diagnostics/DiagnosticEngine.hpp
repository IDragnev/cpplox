#pragma once

#include "cpplox/diagnostics/DiagnosticConsumer.hpp"
#include "cpplox/diagnostics/Diagnostic.hpp"

#include <fmt/format.h>

namespace cpplox {
    class DiagnosticEngine {
    public:
        explicit DiagnosticEngine(DiagnosticConsumer* c)
            : consumer(c) {}
        DiagnosticEngine(DiagnosticEngine&&) = default;
        ~DiagnosticEngine() = default;

        DiagnosticEngine& operator=(DiagnosticEngine&&) = default;

        template <typename... Args>
        void report(std::size_t line, std::string_view fmtStr, Args&&... args) {
            consumer->consume(Diagnostic{
                .msg = String(fmt::vformat(fmtStr, fmt::make_format_args(args...))),
                .line = line,
            });
        }

    private:
        DiagnosticConsumer* consumer = nullptr;
    };
} // namespace cpplox