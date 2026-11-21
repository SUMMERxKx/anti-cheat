#include "toolchain_c.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
#error "The boundary layer must be compiled as C99 or later"
#endif

long ac_toolchain_c_standard(void)
{
    return __STDC_VERSION__;
}

int ac_toolchain_c_lab_build(void)
{
#if defined(LAB_BUILD) && LAB_BUILD == 1
    return 1;
#else
    return 0;
#endif
}

float ac_toolchain_c_sum(const float *values, int count)
{
    float total = 0.0f;
    for (int i = 0; i < count; ++i) {
        total += values[i];
    }
    return total;
}
