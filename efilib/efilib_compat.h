/** @file
  Compatibility functions for libc based code

  Copyright (c) 2026, DTI Technologies s.r.o. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef _EFILIB_COMPAT_H_
#define _EFILIB_COMPAT_H_

#include <efi.h>
#include <efilib.h>

typedef UINTN          size_t;

size_t strlen(const char *s);

// void *malloc(size_t size);
// void free(void * p);
// void *calloc(size_t n, size_t size);

void *memset(void * s, int c, size_t n);
void *memcpy(void * dest, const void * src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

#endif
