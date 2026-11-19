#pragma once

// NXDK / libc++ compatibility layer for the Xbox build.
//
// This header is force-included (-include) into every C++ translation unit,
// including libc++ itself, so it must stay self-contained: no engine headers,
// no project macros to test.
//
// 1. NXDK's <__config> defines _LIBCPP_HAS_NO_STDOUT for freestanding Xbox
//    builds, so libc++'s <cstdio> drops the using-declarations for printf and
//    friends.  The engine prints its diagnostics through std::printf.
// 2. libc++ 10's <locale> fallbacks call wcsnrtombs()/mbsnrtowcs().  Their
//    prototypes live in libc++'s own <wchar.h> wrapper, which NXDK's include
//    order never reaches; the definitions are part of libc++.lib
//    (src/support/win32/support.cpp), so declarations are all that is missing.
// 3. libc++ 10's <cmath> uses std::numeric_limits without including <limits>.
// 4. pdclib's Annex K guard header (_PDCLIB_lib_ext1.h) has no include guard
//    and mistakes "still undefined" for "was defined earlier" on the second
//    include.  libc++'s system_error.cpp asks for ext1 itself, so pin the value
//    here (before any header) to the same 1 it uses.

#define __STDC_WANT_LIB_EXT1__ 1

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <limits>

// Repeated using-declarations of the same entity are allowed; when the config
// already imported these the lines are harmless no-ops.
namespace std {
using ::printf;
using ::putchar;
using ::puts;
using ::vprintf;
}  // namespace std

// C++ linkage, matching libc++'s support/win32/support.cpp definitions.
std::size_t wcsnrtombs(char* __restrict dst, const wchar_t** __restrict src,
                       std::size_t max_source_chars,
                       std::size_t dst_size_bytes, mbstate_t* __restrict ps);
std::size_t mbsnrtowcs(wchar_t* __restrict dst, const char** __restrict src,
                       std::size_t max_source_chars,
                       std::size_t dst_size_bytes, mbstate_t* __restrict ps);
