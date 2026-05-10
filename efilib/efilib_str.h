/** @file
  Precise Time functions library

  Copyright (c) 2026, DTI Technologies s.r.o. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef _EFILIB_STR_H_
#define _EFILIB_STR_H_

UINTN StrToUTF8(CHAR8 * Out, UINTN Size, CONST CHAR16 * In);
UINTN StrNToUTF8(CHAR8 * Out, UINTN Size, CONST CHAR16 * In, UINTN Len);

#endif
