/** @file
  Precise Time functions library

  Copyright (c) 2026, DTI Technologies s.r.o. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef _EFILIB_TIME_H_
#define _EFILIB_TIME_H_

typedef UINT64 (EFIAPI *_EFI_TimeGet) (VOID);

extern _EFI_TimeGet   TimeGet;

void EFIAPI TimeInit(IN EFI_HANDLE ImageHandle);

#endif
