/* C99 side of the toolchain check. Shaped like the lab_api.h boundary. */
#ifndef AC_TOOLCHAIN_C_H
#define AC_TOOLCHAIN_C_H

#ifdef __cplusplus
extern "C" {
#endif

/* Returns the value of __STDC_VERSION__ the C file was compiled with. */
long ac_toolchain_c_standard(void);

/* Returns 1 when the C file was compiled with LAB_BUILD defined to 1. */
int ac_toolchain_c_lab_build(void);

/* Sums an array of floats. A trivial check that data crosses the C ABI intact. */
float ac_toolchain_c_sum(const float *values, int count);

#ifdef __cplusplus
}
#endif

#endif /* AC_TOOLCHAIN_C_H */
