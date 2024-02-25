#pragma once
#include <spdlog/spdlog.h>

#define assert_fatal(cond, msg)                                                \
    if (!(cond)) {                                                             \
        spdlog::error(msg);                                                    \
        exit(1);                                                               \
    }

#define assert_fatalf(cond, fmt, ...)                                          \
    if (!(cond)) {                                                             \
        spdlog::error(fmt, __VA_ARGS__);                                       \
        exit(1);                                                               \
    }
