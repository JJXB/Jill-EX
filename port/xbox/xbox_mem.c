// Fast memcpy/memset/memmove for the Xbox build.
//
// NXDK's pdclib ships reference implementations that copy one byte at a time
// (lib/pdclib/functions/string/memcpy.c is literally `while (n--) *d++ = *s++;`).
// That is fine for a reference C library but catastrophic here: presenting a
// 640x400 frame is 1 MB per frame, and the byte loop turned it into a ~30 ms
// cost on a 733 MHz Pentium III (and slowed SDL, libc++ and the audio mixer).
//
// These definitions are linked directly, so the archive members of libpdclib
// are not pulled in.  The bulk moves use `rep movs`/`rep stos`, which the CPU
// turns into full-line writes that the write-combining framebuffer likes.

#include <stddef.h>
#include <stdint.h>

void* memcpy(void* dest, const void* src, size_t n) {
    void* const ret = dest;
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    size_t words = n >> 2;
    if (words != 0) {
        __asm__ __volatile__("rep movsl"
                             : "+D"(d), "+S"(s), "+c"(words)
                             :
                             : "memory");
    }
    size_t tail = n & 3;
    while (tail-- != 0) {
        *d++ = *s++;
    }
    return ret;
}

void* memset(void* s, int c, size_t n) {
    void* const ret = s;
    unsigned char* d = (unsigned char*)s;
    const unsigned char byte = (unsigned char)c;
    uint32_t value = (uint32_t)byte;
    value |= value << 8;
    value |= value << 16;
    size_t words = n >> 2;
    if (words != 0) {
        __asm__ __volatile__("rep stosl"
                             : "+D"(d), "+c"(words)
                             : "a"(value)
                             : "memory");
    }
    size_t tail = n & 3;
    while (tail-- != 0) {
        *d++ = byte;
    }
    return ret;
}

void* memmove(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    if (d == s || n == 0) {
        return dest;
    }
    if (d < s || d >= s + n) {
        return memcpy(dest, src, n);
    }
    // Overlapping and moving towards higher addresses: copy backwards.
    d += n;
    s += n;
    while (n != 0) {
        *--d = *--s;
        --n;
    }
    return dest;
}
