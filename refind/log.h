/*
 * refind/log.h
 *
 * Definitions to handle rEFIng's logging facility, activated by setting
 * log_level in refind.conf.
 *
 */
/*
 * Copyright (c) 2012-2024 Roderick W. Smith
 * Copyright (c) 2026, DTI Technologies s.r.o. All rights reserved.
 *
 * Distributed under the terms of the GNU General Public License (GPL)
 * version 3 (GPLv3), a copy of which must be distributed with this source
 * code or binaries made from it.
 *
 */

#ifndef _LOG_H_
#define _LOG_H_

#include "efi.h"
#include "efilib.h"
#define EFI_DEVICE_PATH_PROTOCOL EFI_DEVICE_PATH

#define LOG_LINE_NORMAL      1
#define LOG_LINE_SEPARATOR   2
#define LOG_LINE_THIN_SEP    3

#define LOGFILE L"refing.log"
#define LOGFILE_OLD L"refing.log-old"

#define LOG(Level, Type, ...) LOG_Write(Level, Type, __VA_ARGS__)

EFI_STATUS LOG_Init();
VOID LOG_Activate(BOOLEAN Direct);
VOID LOG_Reactivate();
VOID LOG_Flush();
VOID LOG_End();

VOID LOG_Write(UINTN Level, UINTN Type, IN CONST CHAR16 *Format, ...);

#endif
