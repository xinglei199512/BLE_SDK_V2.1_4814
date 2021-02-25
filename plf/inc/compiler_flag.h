#ifndef COMPILER_FLAG_H_
#define COMPILER_FLAG_H_

#if !defined(__BOOT_RAM__)
#define USED_FLAG __attribute__((used))
#define XIP_SECTION __attribute__((section("xip_section")))
#define N_XIP_SECTION __attribute__((section("n_xip_section")))
#define N_XIP_VARIABLE __attribute__((section("n_xip_variable")))
#define NOINLINE __attribute__((noinline))
#else
#define USED_FLAG
#define XIP_SECTION
#define N_XIP_SECTION
#define N_XIP_VARIABLE
#define NOINLINE
#endif
#endif
