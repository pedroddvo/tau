#pragma once
#include <stdlib.h>
#include "log.h"

#define assert_fatal(cond, msg)                                                \
    if (!(cond)) {                                                             \
        log_info(msg);                                                         \
        exit(1);                                                               \
    }

#define assert_fatalf(cond, msg, ...)                                          \
    if (!(cond)) {                                                             \
        log_info(msg, __VA_ARGS__);                                            \
        exit(1);                                                               \
    }
