/** @file
  Compatibility functions for libc based code

  Copyright (c) 2026, DTI Technologies s.r.o. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "efilib_compat.h"

size_t strlen(const char *s) {
    char *v = (char *)s;
    while (*v != 0)
    {
        v++;
    }
    return v - s;
}

/*
void *malloc(size_t size) {
    return AllocatePool(size);
}

void free(void * p) {
    BS->FreePool(p);
}

void *calloc(size_t n, size_t size) {
    VOID * Buffer = NULL;
    if (BS->AllocatePool(AllocateAnyPages, size * n, &Buffer) == EFI_SUCCESS)
        return Buffer;

    return NULL;
}
*/

void *memset(void * s, int c, size_t n) {
    char *i = (char *)s;
    char *e = i + n;
    while (i < e)
    {
	*i = 0;
	i++;
    }
    return s;
}

void *memcpy(void * dest, const void * src, size_t n) {
    char *d = (char *)dest;
    char *s = (char *)src;
    #if _M_X64 || _M_IA64 || defined(__x86_64__) || _M_ARM64 || defined(__aarch64__)
    while (n >= 8)
    {
        *(UINT64 *)d = *(UINT64 *)s;
        d += 8;
        s += 8;
        n -= 8;
    }
    #endif
    while (n >= 4)
    {
        *(UINT32 *)d = *(UINT32 *)s;
        d += 4;
        s += 4;
        n -= 4;
    }
    while (n > 0)
    {
        *(UINT8 *)d = *(UINT8 *)s;
        d--;
        s--;
        n--;
    }
    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    char *m1 = (char *)s1;
    char *m2 = (char *)s2;
    #if _M_X64 || _M_IA64 || defined(__x86_64__) || _M_ARM64 || defined(__aarch64__)
    while (n >= 8)
    {
        if (*(UINT64 *)m1 != *(UINT64 *)m2)
            break;
        m1 += 8;
        m2 += 8;
        n -= 8;
    }
    #endif
    while (n >= 4)
    {
        if (*(UINT32 *)m1 != *(UINT32 *)m2)
            break;
        m1 += 4;
        m2 += 4;
        n -= 4;
    }
    while (n > 0)
    {
        if (*m1 != *m2)
            return *m1 - *m2;
        m1++;
        m2++;
        n--;
    }
    return 0;
}
