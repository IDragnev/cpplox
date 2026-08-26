#pragma once

#include <cstdio>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

namespace cpplox {
    inline bool isTerminal(FILE* stream) {
#ifdef _WIN32
        return _isatty(_fileno(stream)) != 0;
#else
        return isatty(fileno(stream)) != 0;
#endif
    }

    inline const bool STDIN_IS_TERMINAL = isTerminal(stdin);
    inline const bool STDERR_IS_TERMINAL = isTerminal(stderr);
} // namespace cpplox
