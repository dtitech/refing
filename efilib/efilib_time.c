/** @file
  Precise Time functions library

  Copyright (c) 2026, DTI Technologies s.r.o. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifdef __MAKEWITH_GNUEFI
#include "efi.h"
#include "efilib.h"
#endif
#include "efilib_time.h"
#include "../include/refit_call_wrapper.h"
#include "efilib_acpi.h"
#include <x86gprintrin.h>

#define EFI_TIMESTAMP_PROTOCOL_GUID { 0xafbfde41, 0x2e6e, 0x4262, { 0xba, 0x65, 0x62, 0xb9, 0x23, 0x6e, 0x54, 0x95 }}

#pragma pack(push,1)

typedef struct {
  UINT64                      Frequency;
  UINT64                      EndValue;
} EFI_TIMESTAMP_PROPERTIES;

typedef UINT64 (EFIAPI *TIMESTAMP_GET) (VOID);
typedef EFI_STATUS (EFIAPI *TIMESTAMP_GET_PROPERTIES) (OUT EFI_TIMESTAMP_PROPERTIES *Properties);

typedef struct _EFI_TIMESTAMP_PROTOCOL {
  TIMESTAMP_GET               GetTimestamp;
  TIMESTAMP_GET_PROPERTIES    GetProperties;
} EFI_TIMESTAMP_PROTOCOL;

typedef struct ACPI_RSDT
{
  EFI_ACPI_DESCRIPTION_HEADER Header;
  UINT32                      Ptr[];
} ACPI_RSDT;

typedef struct ACPI_XSDT
{
  EFI_ACPI_DESCRIPTION_HEADER Header;
  UINT64                      Ptr[];
} ACPI_XSDT;

#pragma pack(pop)

#define __CDEINTERRUPT_DISABLE __asm__ volatile("cli");
#define __CDEINTERRUPT_ENABLE __asm__ volatile("sti");

const UINT8 dtACPI_RsdPtr[] = { 0x52, 0x53, 0x44, 0x20, 0x50, 0x54, 0x52, 0x20 };

EFI_GUID gTimestampGuid = EFI_TIMESTAMP_PROTOCOL_GUID;

UINT64                   gTimeIV   = 0;      // Time initial value (rEFInd start time)
UINTN                    gTimeDiv  = 1;      // Time divisor (TSC)
UINTN                    gTimeOp   = 0;      // Time operation 0 = divide, 1 = multiply
UINTN                    gTimeMod  = 1;      // Time operation value

EFI_TIMESTAMP_PROTOCOL  *gTSP      = NULL;

UINT64                   gInitTSC  = 0;

UINT64 EFIAPI ETIME_GetTime();

_EFI_TimeGet             TimeGet = ETIME_GetTime;

int IsEqualGUID(const EFI_GUID *GUID1, const EFI_GUID *GUID2) {
    if ((((UINT64 *)GUID1)[0] == ((UINT64 *)GUID2)[0]) &&
        (((UINT64 *)GUID1)[1] == ((UINT64 *)GUID2)[1]))
        return 0;

    return -1;
}

unsigned int __indword(unsigned short p) {
    unsigned int v;
    __asm__ __volatile__("inl %w[p], %k[v]": [v] "=a" (v) : [p] "Nd" (p));
    return v;
}

int ACPI_CheckXSDP(EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER *XSDP) {
    if (XSDP->Signature != *(UINT64 *)dtACPI_RsdPtr)
        return -1;

    if (XSDP->Revision < 2)
        return -1;

    const UINT8 *i_Buffer = (UINT8 *)XSDP;
    UINT8 i_ChkSum = 0;
    for (UINTN i_Pos = 0; i_Pos < sizeof(EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER); i_Pos++)
        i_ChkSum += i_Buffer[i_Pos];

    if (i_ChkSum != 0)
        return -1;

    return 0;
}

EFI_STATUS ACPI_GetXSDT(ACPI_XSDT **XSDT) {
    const EFI_GUID i_AT_GUID = EFI_ACPI_TABLE_GUID;
    const UINTN i_Count = ST->NumberOfTableEntries;
    EFI_CONFIGURATION_TABLE *i_CT = ST->ConfigurationTable;

    for (UINTN i_Pos = 0; i_Pos < i_Count; i_Pos++) {
        if (IsEqualGUID(&(i_CT[i_Pos].VendorGuid), &i_AT_GUID) == 0) {
            EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER *i_XSDP = (EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER *)(i_CT[i_Pos].VendorTable);

            if (ACPI_CheckXSDP(i_XSDP) == 0) {
                *XSDT = (ACPI_XSDT *)(UINTN)i_XSDP->XsdtAddress;
                return EFI_SUCCESS;
            }
        }
    }

    *XSDT = NULL;
    return EFI_NOT_FOUND;
}

static uint16_t GetACPICount(uint16_t AcpiPmTmrBase) {
    return UINT16_MAX & (unsigned long)__indword(AcpiPmTmrBase); // NOTE: PmTmr requires 32Bit read!
}

UINT64 GetTscPerSec(uint16_t AcpiPmTmrBase) {
#define CLKWAIT (62799 * 3)
    int64_t  count = CLKWAIT;
    uint64_t qwTSCPerIntervall = 100000, qwTSCEnd, qwTSCStart, qwTicksGoneThrough;
    size_t eflags = __readeflags();                         // save flags

    __CDEINTERRUPT_DISABLE;
    GetACPICount(AcpiPmTmrBase);

    if (1) {
        uint16_t previous, current, diff = 0;

        previous = GetACPICount(AcpiPmTmrBase);
        qwTSCStart = __rdtsc();                             // get TSC start

        while (count > 0) {
            current = GetACPICount(AcpiPmTmrBase);
            if (current >= previous)
                diff = current - previous;
            else
                diff = ~(previous - current) + 1;

            previous = current;
            count -= diff;
        }

        qwTSCEnd = __rdtsc();                               // get TSC end ~50ms
        qwTicksGoneThrough = qwTSCEnd - qwTSCStart;         // calculate the additional number of TSC

        //
        //     qwTicksGoneThrough     qwTSCPerIntervall
        //     ------------------ == -------------------      -->
        //      CLKWAIT - count          CLKWAIT
        //
        //
        //                           qwTicksGoneThrough * CLKWAIT
        //      qwTSCPerIntervall = -----------------------------
        //                               CLKWAIT - count
        //
        //          NOTE: "count" is negative. " - count " ADDs additional ticks gone through
        //
        qwTSCPerIntervall = ((qwTicksGoneThrough) * CLKWAIT) / (CLKWAIT - count);    // get number of CPU TSC per ACPI PmTmr ClkTick (3.57MHz)

        if (0x200 & eflags)                                 // restore IF interrupt flag
            __CDEINTERRUPT_ENABLE;
    }

    return (int64_t) 19 * qwTSCPerIntervall;
}

UINT64 EFIAPI TSC_GetTime() {
    UINT64 Out, Stamp, Frac;

    Stamp = __rdtsc();
    Frac = Stamp % gTimeDiv;

    Out = (Stamp / gTimeDiv) * 1000;
    if (gTimeOp) {
        Out += Frac / gTimeMod;
    } else {
        Out += Frac * gTimeMod;
    }

    return Out - gTimeIV;
}

UINT64 EFIAPI TSP_GetTime() {
    UINT64 Out, Stamp, Frac;

    Stamp = gTSP->GetTimestamp();
    Frac = Stamp % gTimeDiv;

    Out = (Stamp / gTimeDiv) * 1000;
    if (gTimeOp) {
        Out += Frac / gTimeMod;
    } else {
        Out += Frac * gTimeMod;
    }

    return Out - gTimeIV;
}

UINT64 EFIAPI ETIME_GetTime() {
    UINT64 Out;
    EFI_STATUS Status;
    EFI_TIME Time;

    Status = refit_call2_wrapper(RT->GetTime, &Time, NULL);
    if (Status == EFI_SUCCESS) {
        Out = (Time.Hour * 3600000) + (Time.Minute * 60000) + (Time.Second * 1000) +
              (Time.Nanosecond / 1000000);
    } else {
        Out = 0;
    }

    return Out - gTimeIV;
}

EFI_STATUS TSC_Init(IN EFI_HANDLE ImageHandle) {
    unsigned short AcpiPmTmrBase = UINT16_MAX;
    uint32_t idFACP = 0x50434146lu;
    ACPI_XSDT *pXSDT = NULL;
    EFI_ACPI_6_2_FIXED_ACPI_DESCRIPTION_TABLE* pFACP;
    char foundTbl = 0;

    if (ACPI_GetXSDT(&pXSDT) != EFI_SUCCESS)
        return EFI_UNSUPPORTED;

    UINTN uCount = pXSDT->Header.Length - sizeof(ACPI_XSDT);

    for (UINTN ixTbl = 0; ixTbl < uCount; ixTbl++) {
        EFI_ACPI_DESCRIPTION_HEADER *pTbl = (EFI_ACPI_DESCRIPTION_HEADER *)(UINTN)pXSDT->Ptr[ixTbl];
        if (pTbl->Signature == idFACP) {
            pFACP = (void *)pTbl;
            foundTbl = 1;
            AcpiPmTmrBase = (unsigned short)pFACP->PmTmrBlk;
            break;
        }
    }

    if (foundTbl == 0)
        return EFI_UNSUPPORTED;

    gInitTSC = __rdtsc();
    gTimeDiv = GetTscPerSec(AcpiPmTmrBase);
    if (gTimeDiv < 1000) {
        gTimeOp = 0;
        gTimeMod = 1000 / gTimeDiv;
    } else {
        gTimeOp = 1;
        gTimeMod = gTimeDiv / 1000;
    }
    gTimeIV = TSC_GetTime();
    TimeGet = TSC_GetTime;

    return EFI_SUCCESS;
}

EFI_STATUS TSP_Init(IN EFI_HANDLE ImageHandle) {
    EFI_STATUS Status;
    EFI_TIMESTAMP_PROPERTIES Props;

    Status = refit_call3_wrapper(BS->HandleProtocol, ImageHandle, &gTimestampGuid, (VOID **) &gTSP);
    if (Status != EFI_SUCCESS)
        return Status;

    Status = gTSP->GetProperties(&Props);
    if ((Status != EFI_SUCCESS) || (Props.Frequency == 0)) {
        gTSP = NULL;
        return Status;
    }

    gTimeDiv = Props.Frequency;
    if (Props.Frequency < 1000) {
        gTimeOp = 0;
        gTimeMod = 1000 / Props.Frequency;
    } else {
        gTimeOp = 1;
        gTimeMod = Props.Frequency / 1000;
    }

    gTimeIV = TSP_GetTime();
    TimeGet = TSP_GetTime;

    return EFI_SUCCESS;
}

EFI_STATUS ETIME_Init(IN EFI_HANDLE ImageHandle) {
    EFI_STATUS Status;
    EFI_TIME Time;
    EFI_TIME_CAPABILITIES Caps;

    Status = refit_call2_wrapper(RT->GetTime, &Time, &Caps);
    if (Status != EFI_SUCCESS) {
        return Status;
    }

    gTimeIV = (Time.Hour * 3600000) + (Time.Minute * 60000) + (Time.Second * 1000) +
              (Time.Nanosecond / 1000000);
    TimeGet = ETIME_GetTime;

    return EFI_SUCCESS;
}

VOID EFIAPI TimeInit(IN EFI_HANDLE ImageHandle) {
    if (TSP_Init(ImageHandle) == EFI_SUCCESS)
        return;

    if (TSC_Init(ImageHandle) == EFI_SUCCESS)
        return;

    ETIME_Init(ImageHandle);
}
