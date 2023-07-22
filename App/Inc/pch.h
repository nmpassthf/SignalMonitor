/**
 * @file pch.h
 * @author nmpassthf (nmpassthf@gmail.com)
 * @brief
 * @date 2023-07-15
 *
 * @copyright Copyright (c) nmpassthf 2023
 *
 */
#ifndef __M_PCH_H__
#define __M_PCH_H__

#include <chrono>
#include <iostream>

#include "config.h"

constexpr auto PROGRAM_NAME = "Signal Monitor";
constexpr auto VERSION = signalmonitor_VERSION;

constexpr inline bool floatIsZero(const float &f) {
    return f <= FLT_EPSILON && f >= -FLT_EPSILON;
}
constexpr inline bool floatIsZero(const double &f) {
    return f <= DBL_EPSILON && f >= -DBL_EPSILON;
}
constexpr inline bool floatIsOverflow(const float &f) {
    return f >= FLT_MAX || f <= -FLT_MAX;
}
constexpr inline bool floatIsOverflow(const double &f) {
    return f >= DBL_MAX || f <= -DBL_MAX;
}

/**
 * @brief macros
 *
 */

#define _INITIALIZER(f)                                        \
    static void f(void);                                       \
                                                               \
    struct __INITIALIZER_##f##_TYPE {                          \
        __INITIALIZER_##f##_TYPE(void) { f(); }                \
    };                                                         \
    static __INITIALIZER_##f##_TYPE __INITIALIZER##f##__CLASS; \
    void f(void)

#define _DESTROYER(f)                                      \
    static void f(void);                                   \
                                                           \
    struct __DESTROYER_##f##_TYPE {                        \
        __DESTROYER_##f##_TYPE(void) { atexit(f); }        \
    };                                                     \
    static __DESTROYER_##f##_TYPE __DESTROYER##f##__CLASS; \
    void f(void)

#define INITIALIZER(FUNC_NAME) _INITIALIZER(FUNC_NAME)
#define DESTROYER(FUNC_NAME) _DESTROYER(FUNC_NAME)

#include <QDebug>
inline auto printCurrentTime() {
    std::stringstream ss;
    ss << "[" << std::chrono::system_clock::now() << "]";
    return qDebug() << ss.str().c_str();
}

#endif /* __M_PCH_H__ */
