// Checks that the toolchain builds C99 and C++20 side by side and that a C99
// function can be called from C++ through a plain C ABI.

#include "toolchain_c.h"

#include <array>
#include <cstdio>

#if __cplusplus < 202002L && !(defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)
#error "Our C++ code must be compiled as C++20 or later"
#endif

namespace {

int failures = 0;

void check(bool ok, const char *what)
{
    std::printf("%s: %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) {
        ++failures;
    }
}

} // namespace

int main()
{
    check(ac_toolchain_c_standard() == 199901L, "C file is compiled as C99");
    check(ac_toolchain_c_lab_build() == 1, "C file sees LAB_BUILD=1");

#if defined(LAB_BUILD) && LAB_BUILD == 1
    check(true, "C++ file sees LAB_BUILD=1");
#else
    check(false, "C++ file sees LAB_BUILD=1");
#endif

    constexpr std::array<float, 4> values{1.0f, 2.0f, 3.5f, -0.5f};
    check(ac_toolchain_c_sum(values.data(), static_cast<int>(values.size())) == 6.0f,
          "float array crosses the C ABI intact");

    return failures == 0 ? 0 : 1;
}
