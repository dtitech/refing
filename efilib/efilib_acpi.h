/** @file
  ACPI definitions from the ACPI Specification

  Copyright (c) 2006 - 2025, Intel Corporation. All rights reserved.
  Copyright (c) 2019 - 2024, Arm Limited. All rights reserved.
  Copyright (c) 2023, Loongson Technology Corporation Limited. All rights reserved.
  Copyright (C) 2025, Advanced Micro Devices, Inc. All rights reserved.
  Copyright (c) 2026, DTI Technologies s.r.o. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef _MYTIME_ACPI_H_
#define _MYTIME_ACPI_H_

#define EFI_ACPI_TABLE_GUID { 0x8868e871, 0xe4f1, 0x11d3, {0xbc, 0x22, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 } }

//
// ACPI 2.0 or newer tables should use EFI_ACPI_TABLE_GUID.
//
#define EFI_ACPI_20_TABLE_GUID EFI_ACPI_TABLE_GUID

//
// Ensure proper structure formats
//
#pragma pack(1)

///
/// The common ACPI description table header.  This structure prefaces most ACPI tables.
///
typedef struct {
  UINT32  Signature;
  UINT32  Length;
  UINT8   Revision;
  UINT8   Checksum;
  UINT8   OemId[6];
  UINT64  OemTableId;
  UINT32  OemRevision;
  UINT32  CreatorId;
  UINT32  CreatorRevision;
} EFI_ACPI_DESCRIPTION_HEADER; 

///
/// Root System Description Pointer Structure
///
typedef struct {
  UINT64  Signature;
  UINT8   Checksum;
  UINT8   OemId[6];
  UINT8   Revision;
  UINT32  RsdtAddress;
  UINT32  Length;
  UINT64  XsdtAddress;
  UINT8   ExtendedChecksum;
  UINT8   Reserved[3];
} EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER;

///
/// Common table header, this prefaces all ACPI tables, including FACS, but
/// excluding the RSD PTR structure
///
typedef struct {
  UINT32  Signature;
  UINT32  Length;
} EFI_ACPI_2_0_COMMON_HEADER;

///
/// ACPI 6.2 Generic Address Space definition
///
typedef struct {
  UINT8     AddressSpaceId;
  UINT8     RegisterBitWidth;
  UINT8     RegisterBitOffset;
  UINT8     AccessSize;
  UINT64    Address;
} EFI_ACPI_6_2_GENERIC_ADDRESS_STRUCTURE;

///
/// Fixed ACPI Description Table Structure (FADT)
///
typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER               Header;
  UINT32                                    FirmwareCtrl;
  UINT32                                    Dsdt;
  UINT8                                     Reserved0;
  UINT8                                     PreferredPmProfile;
  UINT16                                    SciInt;
  UINT32                                    SmiCmd;
  UINT8                                     AcpiEnable;
  UINT8                                     AcpiDisable;
  UINT8                                     S4BiosReq;
  UINT8                                     PstateCnt;
  UINT32                                    Pm1aEvtBlk;
  UINT32                                    Pm1bEvtBlk;
  UINT32                                    Pm1aCntBlk;
  UINT32                                    Pm1bCntBlk;
  UINT32                                    Pm2CntBlk;
  UINT32                                    PmTmrBlk;
  UINT32                                    Gpe0Blk;
  UINT32                                    Gpe1Blk;
  UINT8                                     Pm1EvtLen;
  UINT8                                     Pm1CntLen;
  UINT8                                     Pm2CntLen;
  UINT8                                     PmTmrLen;
  UINT8                                     Gpe0BlkLen;
  UINT8                                     Gpe1BlkLen;
  UINT8                                     Gpe1Base;
  UINT8                                     CstCnt;
  UINT16                                    PLvl2Lat;
  UINT16                                    PLvl3Lat;
  UINT16                                    FlushSize;
  UINT16                                    FlushStride;
  UINT8                                     DutyOffset;
  UINT8                                     DutyWidth;
  UINT8                                     DayAlrm;
  UINT8                                     MonAlrm;
  UINT8                                     Century;
  UINT16                                    IaPcBootArch;
  UINT8                                     Reserved1;
  UINT32                                    Flags;
  EFI_ACPI_6_2_GENERIC_ADDRESS_STRUCTURE    ResetReg;
  UINT8                                     ResetValue;
  UINT16                                    ArmBootArch;
  UINT8                                     MinorVersion;
  UINT64                                    XFirmwareCtrl;
  UINT64                                    XDsdt;
  EFI_ACPI_6_2_GENERIC_ADDRESS_STRUCTURE    XPm1aEvtBlk;
  EFI_ACPI_6_2_GENERIC_ADDRESS_STRUCTURE    XPm1bEvtBlk;
  EFI_ACPI_6_2_GENERIC_ADDRESS_STRUCTURE    XPm1aCntBlk;
  EFI_ACPI_6_2_GENERIC_ADDRESS_STRUCTURE    XPm1bCntBlk;
  EFI_ACPI_6_2_GENERIC_ADDRESS_STRUCTURE    XPm2CntBlk;
  EFI_ACPI_6_2_GENERIC_ADDRESS_STRUCTURE    XPmTmrBlk;
  EFI_ACPI_6_2_GENERIC_ADDRESS_STRUCTURE    XGpe0Blk;
  EFI_ACPI_6_2_GENERIC_ADDRESS_STRUCTURE    XGpe1Blk;
  EFI_ACPI_6_2_GENERIC_ADDRESS_STRUCTURE    SleepControlReg;
  EFI_ACPI_6_2_GENERIC_ADDRESS_STRUCTURE    SleepStatusReg;
  UINT64                                    HypervisorVendorIdentity;
} EFI_ACPI_6_2_FIXED_ACPI_DESCRIPTION_TABLE;

//
// Return default alignment
//
#pragma pack()

#endif
