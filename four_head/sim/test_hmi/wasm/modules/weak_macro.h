/* weak_macro.h — 跨编译器 __weak 兼容宏 */
#ifndef WEAK_MACRO_H
#define WEAK_MACRO_H

#ifdef __ARMCC_VERSION
#define WEAK __weak
#else
#define WEAK __attribute__((weak))
#endif

#endif
