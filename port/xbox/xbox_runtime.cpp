// Minimal C++ exception runtime for the Xbox build.
//
// NXDK's libxboxrt ships the MSVC <exception> implementation (eh.obj), but it
// is compiled with exceptions enabled, so it references ___CxxFrameHandler3 /
// ___std_terminate -- machinery a freestanding -fno-exceptions Xbox build
// cannot link.  libc++'s own exception.cpp selects that MSVC implementation
// too, so the Xbox target leaves exception.cpp (and the std::future /
// std::thread / mutex translation units that need std::exception_ptr) out of
// libc++ and provides the three symbols the rest of libc++ actually calls.
//
// libc++ declares these three in plain namespace std (not the inline __1
// namespace), which is why no _LIBCPP_BEGIN_NAMESPACE_STD helper is used here.
//
// Nothing in the engine throws: this is the "cannot happen" path from
// libc++'s <ostream>/<ios> state handling.

#include <cstdlib>

#include <__config>

namespace std {

void terminate() noexcept { std::abort(); }

bool uncaught_exception() noexcept { return false; }

int uncaught_exceptions() noexcept { return 0; }

}  // namespace std

// libc++'s mutex.cpp is not built (see port/xbox/CMakeLists.txt), but locale's
// facet registration uses the once-flag helper.  The engine is single
// threaded, so running the initialiser the first time it is seen is exact.
_LIBCPP_BEGIN_NAMESPACE_STD

void __call_once(volatile unsigned long& flag, void* arg,
                 void (*func)(void*)) {
    if (flag == 0) {
        flag = 1;
        func(arg);
    }
}

_LIBCPP_END_NAMESPACE_STD
