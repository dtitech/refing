/*
 * refind/log.c
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

#include "log.h"
#include "global.h"
#include "install.h"
#include "lib.h"
//#include "mystrings.h"
#include "../include/refit_call_wrapper.h"
//#include "screen.h"
//#include "menu.h"
#include "../efilib/efilib_platform.h"
#include "../efilib/efilib_str.h"
#include "../efilib/efilib_time.h"

// Defines

#define LOG_MODE_OFF          0
#define LOG_MODE_INIT         1
#define LOG_MODE_BUFFER       2
#define LOG_MODE_DIRECT       3

#define LOG_BUFFER_PAGE       4096
#define LOG_BUFFER_BLOCK      65536

#define LOG_TMP_CALC_SIZE(Need) ((((Need) + (LOG_BUFFER_BLOCK - 1)) / LOG_BUFFER_BLOCK) * LOG_BUFFER_BLOCK)
#define LOG_BUF_CALC_SIZE(Need) ((((Need) + (LOG_BUFFER_BLOCK - 1)) / LOG_BUFFER_BLOCK) * LOG_BUFFER_BLOCK)

// Typedefs

typedef struct _LOG_Buffer {
    UINTN        End;
    UINTN        Size;
    CHAR8       *Data;
} LOG_Buffer;

typedef struct _LOG_Rec {
    UINT32       Level;
    UINT32       Len;
    CHAR8        Data[];
} LOG_Rec;

// Variables

const CHAR16 *vSEP_THICK_B = L"\n==========";
const CHAR16 *vSEP_THICK_E = L"==========\n";
const CHAR16 *vSEP_THIN_B  = L"\n----------";
const CHAR16 *vSEP_THIN_E  = L"----------\n";

UINTN            gLOG_Mode    = 0;
EFI_FILE_HANDLE  gLOG_Hnd     = NULL;
LOG_Buffer       gLOG_Tmp     = { 0, 0, NULL };
LOG_Buffer       gLOG_Buf     = { 0, 0, NULL };

// Buffer operations

EFI_STATUS LOG_Buffer_Resize(LOG_Buffer *Buf, UINTN Size) {
    if (Buf->Data) {
        if (Buf->Size == Size)
            return EFI_SUCCESS;
        else if ((Buf->Size < Size) || (Buf->End <= Size)) {
            CHAR8 *NewData = AllocatePool(Size);
            if (!NewData)
                return EFI_OUT_OF_RESOURCES;

            memcpy(NewData, Buf->Data, Buf->End);
            FreePool(Buf->Data);
            Buf->Data = NewData;
            Buf->Size = Size;
        } else if (Buf->End > Size)
            return EFI_BAD_BUFFER_SIZE;
    } else {
        Buf->Data = AllocatePool(Size);
        if (!Buf->Data)
            return EFI_OUT_OF_RESOURCES;
        Buf->Size = Size;
    }

    return EFI_SUCCESS;
}

VOID LOG_Buffer_Free(LOG_Buffer * Buf) {
    if (Buf->Data) {
        FreePool(Buf->Data);
        memset(Buf, 0, sizeof(LOG_Buffer));
    }
}

UINTN LOG_Buffer_WriteTLog(LOG_Buffer *Buf, UINTN Level, CONST CHAR16 *Text) {
    const UINTN Len = StrLen(Text);
    LOG_Rec * Rec;
    UINTN OLen;
    UINTN OSize;

    if (Len == 0)
        return 0;

    for (;;)
    {
        const UINTN Free = Buf->Size - Buf->End;
        Rec = (LOG_Rec *)&(Buf->Data[Buf->End]);
        CHAR8 * RecText = Rec->Data;

        OLen = StrNToUTF8(RecText, Free - sizeof(LOG_Rec), Text, Len);
        OSize = sizeof(LOG_Rec) + OLen + 1;

        if (OSize > Free)
        {
            const UINTN NewSize = LOG_BUF_CALC_SIZE(Buf->End + OSize);
            if (LOG_Buffer_Resize(Buf, NewSize) != EFI_SUCCESS)
                return 0;

            continue;
        }

        break;
    }

    Rec->Level = (UINT32)Level;
    Rec->Len = (UINT32)OLen;
    Buf->End += OSize;
    return OLen;
}

UINTN LOG_Buffer_WriteWStr(LOG_Buffer *Buf, CONST CHAR16 *Text) {
    const UINTN Len = StrLen(Text);
    const UINTN Size = Len * sizeof(CHAR16);
    const UINTN Free = Buf->Size - Buf->End;

    if (Size == 0)
        return 0;

    if (Size + sizeof(CHAR16) > Free)
    {
        const UINTN NewSize = LOG_BUF_CALC_SIZE(Buf->End + Size + sizeof(CHAR16));
        if (LOG_Buffer_Resize(Buf, NewSize) != EFI_SUCCESS)
            return 0;
    }

    memcpy(&(Buf->Data[Buf->End]), Text, Size + sizeof(CHAR16));
    Buf->End += Size;
    return Len;
}

UINTN LOG_Buffer_WriteStr(LOG_Buffer *Buf, CONST CHAR16 *Text) {
    const UINTN Len = StrLen(Text);
    UINTN OLen;

    if (Len == 0)
        return 0;

    for (;;)
    {
        const UINTN Free = Buf->Size - Buf->End;
        OLen = StrNToUTF8(Buf->Data + Buf->End, Free, Text, Len);

        const UINTN OSize = OLen + 1;
        if (OSize > Free)
        {
            const UINTN NewSize = LOG_BUF_CALC_SIZE(Buf->End + OSize);
            if (LOG_Buffer_Resize(Buf, NewSize) != EFI_SUCCESS)
                return 0;

            continue;
        }

        break;
    }

    Buf->End += OLen;
    return OLen;
}

UINTN LOG_Buffer_WriteWStrFV(LOG_Buffer *Buf, CONST CHAR16 *Format, VA_LIST Va) {
    const UINTN Space = (Buf->Size - Buf->End) - sizeof(CHAR16);

    const UINTN Len = UnicodeVSPrint((CHAR16 *)(Buf->Data + Buf->End), Space, Format, Va);
    if (Len == 0)
        return 0;

    Buf->End += Len * sizeof(CHAR16);
    *((CHAR16 *)(Buf->Data + Buf->End)) = 0;
    return Len;
}

VOID LOG_Buffer_ConvertTLog(LOG_Buffer *Buf, UINTN Level) {
    UINTN TPos = 0;
    const UINTN TSize = Buf->End;

    Buf->End = 0;

    while (TPos < TSize) {
        const LOG_Rec * Rec = (const LOG_Rec *)&(Buf->Data[TPos]);
        const CHAR8 * RecText = Rec->Data;
        const UINTN Len = Rec->Len;
        TPos += sizeof(LOG_Rec) + (Rec->Len + 1);

        if (Rec->Level <= Level) {
            memcpy(Buf->Data + Buf->End, RecText, Len);
            Buf->End += Len;
            Buf->Data[Buf->End] = 0;
        }
    }
}

// File operations

EFI_STATUS DeleteFile(IN EFI_FILE_PROTOCOL *BaseDir, CHAR16 *FileName) {
    EFI_FILE_HANDLE FileHandle;
    UINT64 FileMode = EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE;
    EFI_STATUS Ret;

    Ret = refit_call5_wrapper(BaseDir->Open, BaseDir, &FileHandle,
                                 FileName, FileMode, 0);

    return (Ret == EFI_SUCCESS) ?
        refit_call1_wrapper(FileHandle->Delete, FileHandle) :
        Ret;
}

// Rename LOGFILE to LOGFILE_OLD. If an error occurs, try to delete LOGFILE
// instead.
// Returns success status (claiming success if log file was deleted rather
// than rotated, or if it doesn't exist to begin with). If unsuccessful,
// logging should be disabled by the calling function.
EFI_STATUS RotateLogFile(EFI_FILE_HANDLE Location) {
    if (FileExists(Location, LOGFILE)) {
        if (FileExists(Location, LOGFILE_OLD))
            DeleteFile(Location, LOGFILE_OLD);

        if (!BackupOldFile(Location, LOGFILE))
            return DeleteFile(Location, LOGFILE);
    }
    return EFI_SUCCESS;
}

// Log operations

VOID LOG_MDK() {
    gLOG_Mode = LOG_MODE_OFF;
    if (gLOG_Hnd) {
        gLOG_Hnd->Close(gLOG_Hnd);
        gLOG_Hnd = NULL;
    }
    LOG_Buffer_Free(&gLOG_Tmp);
    LOG_Buffer_Free(&gLOG_Buf);
}

EFI_STATUS LOG_HndOpen(BOOLEAN Reopen) {
    EFI_STATUS Ret;
    UINT64 FileMode;

    if (Reopen) {
        FileMode = EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE;
    } else {
        FileMode = EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE;
    }
    Ret = refit_call5_wrapper(SelfDir->Open, SelfDir, &gLOG_Hnd, LOGFILE, FileMode, 0);

    return Ret;
}

EFI_STATUS LOG_Init() {
    EFI_STATUS Ret;

    if (gLOG_Tmp.Data == NULL) {
        Ret = LOG_Buffer_Resize(&gLOG_Tmp, LOG_BUFFER_PAGE);
        if (Ret != EFI_SUCCESS)
            return Ret;
    }

    if (gLOG_Buf.Data == NULL) {
        Ret = LOG_Buffer_Resize(&gLOG_Buf, LOG_BUFFER_BLOCK);
        if (Ret != EFI_SUCCESS)
            return Ret;
    }

    gLOG_Mode = LOG_MODE_INIT;
    return EFI_SUCCESS;
}

VOID LOG_Activate(BOOLEAN Direct) {
    if (gLOG_Mode < LOG_MODE_INIT)
        return;

    if (GlobalConfig.LogLevel == 0) {
        LOG_MDK();
        return;
    }

    if (gLOG_Mode == LOG_MODE_INIT) {
        RotateLogFile(SelfDir);
        LOG_Buffer_ConvertTLog(&gLOG_Buf, GlobalConfig.LogLevel);
        gLOG_Mode = LOG_MODE_BUFFER;
    }

    if (gLOG_Hnd == NULL)
        LOG_HndOpen(FALSE);

    if (Direct)
    {
        LOG(3, LOG_LINE_NORMAL, L"Activated DIRECT logging");
        gLOG_Mode = LOG_MODE_DIRECT;
        LOG_Flush();
    }
    else
    {
        LOG(3, LOG_LINE_NORMAL, L"Activated BUFFERED logging");
        gLOG_Mode = LOG_MODE_BUFFER;
    }
}

VOID LOG_Reactivate() {
    if ((gLOG_Mode <= LOG_MODE_INIT) || (GlobalConfig.LogLevel == 0))
        return;

    if ((gLOG_Tmp.Data == NULL) || (gLOG_Buf.Data == NULL))
        return;

    if (gLOG_Hnd == NULL)
        LOG_HndOpen(TRUE);
}

VOID LOG_Flush() {
    if (gLOG_Buf.End > 0) {
        UINTN Size = gLOG_Buf.End;

        if (gLOG_Hnd) {
            refit_call3_wrapper(gLOG_Hnd->Write, gLOG_Hnd, &Size, gLOG_Buf.Data);
            gLOG_Buf.End = 0;
        }
    }
}

VOID LOG_End() {
    LOG_Flush();
    if (gLOG_Hnd)
    {
        refit_call1_wrapper(gLOG_Hnd->Close, gLOG_Hnd);
        gLOG_Hnd = NULL;
    }
}

VOID LOG_WriteD(IN CONST CHAR16 *Text) {
    UINTN Len;

    for (;;) {
        Len = StrToUTF8(gLOG_Buf.Data, gLOG_Buf.Size, Text);
        if (Len < gLOG_Buf.Size)
            break;

        if (LOG_Buffer_Resize(&gLOG_Buf, gLOG_Buf.Size + LOG_BUFFER_BLOCK) != EFI_SUCCESS)
            return;
    }

    if (gLOG_Hnd)
        refit_call3_wrapper(gLOG_Hnd->Write, gLOG_Hnd, &Len, gLOG_Buf.Data);
}

VOID LOG_WriteV(UINTN Level, UINTN Type, IN CONST CHAR16 *Format, IN VA_LIST Va) {
    UINTN ULen;

    if ((!gLOG_Buf.Data) || (!gLOG_Tmp.Data))
        return;

    gLOG_Tmp.End = 0;

    switch (Type) {
        case LOG_LINE_NORMAL:
            {
            UINT64 Time = TimeGet();
            gLOG_Tmp.End = UnicodeSPrint((CHAR16 *)(gLOG_Tmp.Data), gLOG_Tmp.Size, L"%06d.%03d - ", Time / 1000, Time % 1000) * sizeof(CHAR16);
            }
            break;
        case LOG_LINE_SEPARATOR:
            LOG_Buffer_WriteWStr(&gLOG_Tmp, vSEP_THICK_B);
            break;
        case LOG_LINE_THIN_SEP:
            LOG_Buffer_WriteWStr(&gLOG_Tmp, vSEP_THIN_B);
            break;
    }

    ULen = LOG_Buffer_WriteWStrFV(&gLOG_Tmp, Format, Va);
    if (ULen == 0)
        return;

    switch (Type) {
        case LOG_LINE_NORMAL:
            LOG_Buffer_WriteWStr(&gLOG_Tmp, L"\n");
            break;
        case LOG_LINE_SEPARATOR:
            LOG_Buffer_WriteWStr(&gLOG_Tmp, vSEP_THICK_E);
            break;
        case LOG_LINE_THIN_SEP:
            LOG_Buffer_WriteWStr(&gLOG_Tmp, vSEP_THIN_E);
            break;
    }

    switch (gLOG_Mode)
    {
        case LOG_MODE_INIT:
            if ((Type == LOG_LINE_NORMAL) && (Level <= 1))
                uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, (CHAR16 *)gLOG_Tmp.Data);
            LOG_Buffer_WriteTLog(&gLOG_Buf, Level, (CHAR16 *)gLOG_Tmp.Data);
            break;
        case LOG_MODE_BUFFER:
            LOG_Buffer_WriteStr(&gLOG_Buf, (CHAR16 *)gLOG_Tmp.Data);
            break;
        case LOG_MODE_DIRECT:
            LOG_WriteD((CHAR16 *)gLOG_Tmp.Data);
            break;
    }
}

VOID LOG_Write(UINTN Level, UINTN Type, IN CONST CHAR16 *Format, ...) {
    VA_LIST VA;

    if ((gLOG_Mode > LOG_MODE_INIT) && (Type > GlobalConfig.LogLevel))
        return;

    VA_START(VA, Format);
    LOG_WriteV(Level, Type, Format, VA);
    VA_END(VA);
}
