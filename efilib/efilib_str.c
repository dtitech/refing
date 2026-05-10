/** @file
  Precise Time functions library

  Copyright (c) 2026, DTI Technologies s.r.o. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/


#ifdef __MAKEWITH_GNUEFI
#include "efi.h"
#include "efilib.h"
#else
#include "../include/tiano_includes.h"
#endif
#include "efilib_str.h"

//
// Function to convert null terminated string in UCS-2 to UTF-8
//
UINTN StrToUTF8(CHAR8 * Out, UINTN Size, CONST CHAR16 * In)
{
    UINTN OLen = 0;

    while (*In != 0)
    {
        const CHAR16 Cp = *In;
        In++;

        if (Cp < 0x80) {
            if (OLen + 2 < Size) {
                Out[0] = (CHAR8)Cp;
                Out++;
            }
            OLen++;
        } else if (Cp < 0x800) {
            if (OLen + 3 < Size) {
                Out[0] = (CHAR8)(0xC0 | ((Cp & 0x07C0) >> 6));
                Out[1] = (CHAR8)(0x80 |  (Cp & 0x003F));
                Out += 2;
            }
            OLen += 2;
        } else {
            if (OLen + 4 < Size) {
                Out[0] = (CHAR8)(0xE0 | ((Cp & 0xF000) >> 12));
                Out[1] = (CHAR8)(0x80 | ((Cp & 0x0FC0) >> 6));
                Out[2] = (CHAR8)(0x80 |  (Cp & 0x003F));
                Out += 3;
            }
            OLen += 3;
        }
    }

    if (Size > 0)
      *Out = 0;

    return OLen;
}

//
// Function to convert length defined string in UCS-2 to UTF-8
//
UINTN StrNToUTF8(CHAR8 * Out, UINTN Size, CONST CHAR16 * In, UINTN Len)
{
    const CHAR16 * End = In + Len;
    UINTN OLen = 0;

    while (In < End)
    {
        const CHAR16 Cp = *In;
        In++;

        if (Cp < 0x80) {
            if (OLen + 2 < Size) {
                Out[0] = (CHAR8)Cp;
                Out++;
            }
            OLen++;
        } else if (Cp < 0x800) {
            if (OLen + 3 < Size) {
                Out[0] = (CHAR8)(0xC0 | ((Cp & 0x07C0) >> 6));
                Out[1] = (CHAR8)(0x80 |  (Cp & 0x003F));
                Out += 2;
            }
            OLen += 2;
        } else {
            if (OLen + 4 < Size) {
                Out[0] = (CHAR8)(0xE0 | ((Cp & 0xF000) >> 12));
                Out[1] = (CHAR8)(0x80 | ((Cp & 0x0FC0) >> 6));
                Out[2] = (CHAR8)(0x80 |  (Cp & 0x003F));
                Out += 3;
            }
            OLen += 3;
        }
    }

    if (Size > 0)
      *Out = 0;

    return OLen;
}
