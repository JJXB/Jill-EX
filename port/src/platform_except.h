#pragma once

// Small try/catch compatibility shim.
//
// The Xbox (NXDK) target has no C++ unwinder: there is no
// __gxx_personality_v0/_Unwind_Resume in the Xbox CRT, and the Xbox build is
// compiled with -fno-exceptions.  The engine never throws (nothing in port/src
// has a throw statement); the few remaining try/catch blocks only guard
// against a std::filesystem-style failure on other targets.
//
// On Xbox JILL_TRY/JILL_CATCH compile the guarded block as a plain block and
// drop the handler, and JILL_EXC_TEXT() supplies a replacement for `e.what()`
// so the handler body still compiles without ever running.

#if defined(JILL_XBOX)

#define JILL_TRY if (true)
#define JILL_CATCH(decl) \
    (void)0;             \
    if (false)
#define JILL_EXC_TEXT(exc) "exceptions are unavailable on this target"

#else

#define JILL_TRY try
#define JILL_CATCH(decl) catch (decl)
#define JILL_EXC_TEXT(exc) (exc).what()

#endif
