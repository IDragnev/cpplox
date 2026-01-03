#pragma once

#include "cpplox/diagnostics/DiagnosticConsumer.hpp"
#include "cpplox/diagnostics/Diagnostic.hpp"

#include <fmt/format.h>
#include <memory>

namespace cpplox {
    class DiagnosticEngine {
    public:
        DiagnosticEngine(std::unique_ptr<DiagnosticConsumer> c)
            : consumer(std::move(c)) {}
        DiagnosticEngine(DiagnosticEngine&&) = default;
        ~DiagnosticEngine() = default;

        DiagnosticEngine& operator=(DiagnosticEngine&&) = default;

        template <typename... Args>
        void report(std::size_t line, std::string_view fmtStr, Args&&... args) {
            consumer->consume(Diagnostic{
                .msg = fmt::vformat(fmtStr, fmt::make_format_args(args...)),
                .line = line,
            });
        }

    private:
        std::unique_ptr<DiagnosticConsumer> consumer;
    };
} // namespace cpplox::diagnostics