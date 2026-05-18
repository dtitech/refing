/*
 * loader/main.c
 * Main code for the boot menu
 *
 * Copyright (c) 2006-2010 Christoph Pfisterer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *  * Neither the name of Christoph Pfisterer nor the names of the
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Modifications copyright (c) 2012-2024 Roderick W. Smith
 *
 * Modifications distributed under the terms of the GNU General Public
 * License (GPL) version 3 (GPLv3), or (at your option) any later version.
 *
 */
/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "global.h"
#include "config.h"
#include "screen.h"
#include "launch_legacy.h"
#include "lib.h"
#include "icns.h"
#include "install.h"
#include "menu.h"
#include "mok.h"
#include "apple.h"
#include "mystrings.h"
#include "security_policy.h"
#include "driver_support.h"
#include "launch_efi.h"
#include "scan.h"
#include "log.h"
#include "../efilib/efilib_time.h"
#include "../include/refit_call_wrapper.h"
#include "../include/version.h"
#include "../libeg/efiConsoleControl.h"
#include "../libeg/efiUgaDraw.h"

//
// Some built-in menu definitions....

REFIT_MENU_ENTRY MenuEntryReturn   = { L"Return to Main Menu", TAG_RETURN, 1, 0, 0, NULL, NULL, NULL };

REFIT_MENU_SCREEN MainMenu       = { L"Main Menu", NULL, 0, NULL, 0, NULL, 0, L"Automatic boot",
                                     L"Use arrow keys to move cursor; Enter to boot;",
                                     L"Insert, Tab, or F2 for more options; Esc or Backspace to refresh" };
static REFIT_MENU_SCREEN AboutMenu      = { L"About", NULL, 0, NULL, 0, NULL, 0, NULL, L"Press Enter to return to main menu", L"" };

REFIT_CONFIG GlobalConfig = { /* LogDirect = */ FALSE,
                              /* TextOnly = */ FALSE,
                              /* ScanAllLinux = */ TRUE,
                              /* DeepLegacyScan = */ FALSE,
                              /* EnableAndLockVMX = */ FALSE,
                              /* FoldLinuxKernels = */ TRUE,
                              /* EnableMouse = */ FALSE,
                              /* EnableTouch = */ FALSE,
                              /* HiddenTags = */ TRUE,
                              /* UseNvram = */ TRUE,
                              /* ShutdownAfterTimeout = */ FALSE,
                              /* Install = */ FALSE,
                              /* WriteSystemdVars = */ FALSE,
                              /* FollowSymlinks = */ FALSE,
#ifdef EFIAARCH64
                              /* GzippedLoaders = */ TRUE,
#else
                              /* GzippedLoaders = */ FALSE,
#endif
                              /* BackgroundColor = */ FALSE,
                              /* BackgroundPixel = */ { 0, 0, 0, 0 },
                              /* RequestedScreenWidth = */ 0,
                              /* RequestedScreenHeight = */ 0,
                              /* BannerBottomEdge = */ 0,
                              /* RequestedTextMode = */ DONT_CHANGE_TEXT_MODE,
                              /* Timeout = */ 20,
                              /* HideUIFlags = */ 0,
                              /* MaxTags = */ 0,
                              /* GraphicsFor = */ GRAPHICS_FOR_OSX,
                              /* LegacyType = */ LEGACY_TYPE_MAC,
                              /* ScanDelay = */ 0,
                              /* ScreensaverTime = */ 0,
                              /* MouseSpeed = */ 4,
                              /* IconSizes = */ { DEFAULT_BIG_ICON_SIZE / 4,
                                                  DEFAULT_SMALL_ICON_SIZE,
                                                  DEFAULT_BIG_ICON_SIZE,
                                                  DEFAULT_MOUSE_SIZE },
                              /* BannerScale = */ BANNER_NOSCALE,
                              /* LogLevel = */ 0,
                              /* *DiscoveredRoot = */ NULL,
                              /* *SelfDevicePath = */ NULL,
                              /* *BannerFileName = */ NULL,
                              /* *ScreenBackground = */ NULL,
                              /* *ConfigFilename = */ NULL,
                              /* *SelectionSmallFileName = */ NULL,
                              /* *SelectionBigFileName = */ NULL,
                              /* *DefaultSelection = */ NULL,
                              /* *AlsoScan = */ NULL,
                              /* ToolLocations = */ NULL,
                              /* *ExtraToolLocations = */ NULL,
                              /* *DontScanVolumes = */ NULL,
                              /* *DontScanDirs = */ NULL,
                              /* *DontScanFiles = */ NULL,
                              /* *DontScanTools = */ NULL,
                              /* *DontScanFirmware = */ NULL,
                              /* *WindowsRecoveryFiles = */ NULL,
                              /* *MacOSRecoveryFiles = */ NULL,
                              /* *DriverDirs = */ NULL,
                              /* *IconsDir = */ NULL,
                              /* *LinuxPrefixes = */ NULL,
                              /* *LinuxMatchPatterns = */ NULL,
                              /* *ExtraKernelVersionStrings = */ NULL,
                              /* *SpoofOSXVersion = */ NULL,
                              /* CsrValues = */ NULL,
                              /* ShowTools = */ { TAG_SHELL, TAG_MEMTEST, TAG_GDISK, TAG_APPLE_RECOVERY, TAG_WINDOWS_RECOVERY,
                                                  TAG_MOK_TOOL, TAG_ABOUT, TAG_HIDDEN, TAG_SHUTDOWN, TAG_REBOOT, TAG_FIRMWARE,
                                                  TAG_FWUPDATE_TOOL, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                              /* ScanFor = */ 0
                            };

CHAR16 *gHiddenTools = NULL;

EFI_GUID RefindGuid = REFIND_GUID_VALUE;

//
// misc functions
//

VOID AboutrEFIng(VOID)
{
    CHAR16     *FirmwareVendor;
    CHAR16     *TempStr;
    UINT32     CsrStatus;

    LOG(3, LOG_LINE_SEPARATOR, L"Displaying About/Info screen");
    if (AboutMenu.EntryCount == 0) {
        AboutMenu.TitleImage = BuiltinIcon(BUILTIN_ICON_FUNC_ABOUT);
        AddMenuInfoLine(&AboutMenu, PoolPrint(L"rEFIng Version %s", REFING_VERSION));
        AddMenuInfoLine(&AboutMenu, L"");
        AddMenuInfoLine(&AboutMenu, L"Copyright (c) 2006-2010 Christoph Pfisterer");
        AddMenuInfoLine(&AboutMenu, L"Copyright (c) 2012-2024 Roderick W. Smith");
        AddMenuInfoLine(&AboutMenu, L"Copyright (c) 2026, DTI Technologies s.r.o.");
        AddMenuInfoLine(&AboutMenu, L"Portions Copyright (c) Intel Corporation and others");
        AddMenuInfoLine(&AboutMenu, L"Distributed under the terms of the GNU GPLv3 license");
        AddMenuInfoLine(&AboutMenu, L"");
        AddMenuInfoLine(&AboutMenu, L"Running on:");
        FirmwareVendor = StrDuplicate(ST->FirmwareVendor);
        LimitStringLength(FirmwareVendor, MAX_LINE_LENGTH); // More than ~65 causes empty info page on 800x600 display
        AddMenuInfoLine(&AboutMenu, PoolPrint(L" Firmware: %s %d.%02d", FirmwareVendor,
                                              ST->FirmwareRevision >> 16,
                                              ST->FirmwareRevision & ((1 << 16) - 1)));
        AddMenuInfoLine(&AboutMenu, PoolPrint(L" EFI Revision %d.%02d", ST->Hdr.Revision >> 16, ST->Hdr.Revision & ((1 << 16) - 1)));
#if defined(EFI32)
        AddMenuInfoLine(&AboutMenu, PoolPrint(L" Platform: x86 (32 bit); Secure Boot %s %s",
                                              secure_mode() ? L"active" : L"inactive",
                                              ShimLoaded() ? L"(via Shim)" : L"(direct)"));
#elif defined(EFIX64)
        AddMenuInfoLine(&AboutMenu, PoolPrint(L" Platform: x86_64 (64 bit); Secure Boot %s %s",
                                              secure_mode() ? L"active" : L"inactive",
                                              ShimLoaded() ? L"(via Shim)" : L"(direct)"));
#elif defined(EFIAARCH64)
        AddMenuInfoLine(&AboutMenu, PoolPrint(L" Platform: ARM (64 bit); Secure Boot %s %s",
                                              secure_mode() ? L"active" : L"inactive",
                                              ShimLoaded() ? L"(via Shim)" : L"(direct)"));
#else
        AddMenuInfoLine(&AboutMenu, L" Platform: unknown");
#endif
        if (GetCsrStatus(&CsrStatus) == EFI_SUCCESS) {
            RecordgCsrStatus(CsrStatus, FALSE);
            AddMenuInfoLine(&AboutMenu, gCsrStatus);
        }
        TempStr = egScreenDescription();
        AddMenuInfoLine(&AboutMenu, PoolPrint(L" Screen Output: %s", TempStr));
        MyFreePool(TempStr);
        AddMenuInfoLine(&AboutMenu, L"");
        AddMenuInfoLine(&AboutMenu, L"Built with GNU-EFI");
        AddMenuInfoLine(&AboutMenu, L"");
        AddMenuInfoLine(&AboutMenu, L"For more information, see the rEFIng Web site:");
        AddMenuInfoLine(&AboutMenu, L"https://github.com/dtitech/refing");
        AddMenuEntry(&AboutMenu, &MenuEntryReturn);
        MyFreePool(FirmwareVendor);
    }

    RunMenu(&AboutMenu, NULL);
} /* VOID AboutrEFIng() */

// Record the value of the loader's name/description in rEFIng's "PreviousBoot" EFI variable,
// if it's different from what's already stored there.
VOID StoreLoaderName(IN CHAR16 *Name) {

    if (Name) {
        EfivarSetRaw(&RefindGuid, L"PreviousBoot", (CHAR8*) Name, StrLen(Name) * 2 + 2, TRUE);
    } // if
} // VOID StoreLoaderName()

// Rescan for boot loaders
VOID RescanAll(BOOLEAN DisplayMessage, BOOLEAN Reconnect) {
    LOG(3, LOG_LINE_NORMAL, L"Re-scanning all boot loaders");
    FreeList((VOID ***) &(MainMenu.Entries), &MainMenu.EntryCount);
    MainMenu.Entries = NULL;
    MainMenu.EntryCount = 0;
    // ConnectAllDriversToAllControllers() can cause system hangs with some
    // buggy filesystem drivers, so do it only if necessary....
    if (Reconnect) {
        ConnectAllDriversToAllControllers();
        ScanVolumes();
    }
    ReadConfig(GlobalConfig.ConfigFilename);
    SetVolumeIcons();
    ScanForBootloaders(DisplayMessage);
    ScanForTools();
} // VOID RescanAll()

// Set up our own Secure Boot extensions....
// Returns TRUE on success, FALSE otherwise
static BOOLEAN SecureBootSetup(VOID) {
    EFI_STATUS Status;
    BOOLEAN    Success = FALSE;

    LOG(3, LOG_LINE_NORMAL, L"Setting up Secure Boot (if applicable)");
    if (secure_mode() && ShimLoaded()) {
        LOG(2, LOG_LINE_NORMAL, L"Secure boot mode detected with loaded Shim; adding MOK extensions");
        Status = security_policy_install();
        if (Status == EFI_SUCCESS) {
            Success = TRUE;
        } else {
            Print(L"Failed to install MOK Secure Boot extensions");
            PauseForKey();
        }
    } else {
        LOG(2, LOG_LINE_NORMAL, L"Secure boot disabled; doing nothing");
    }
    return Success;
} // VOID SecureBootSetup()

// Remove our own Secure Boot extensions....
// Returns TRUE on success, FALSE otherwise
static BOOLEAN SecureBootUninstall(VOID) {
    EFI_STATUS Status;
    BOOLEAN    Success = TRUE;

    if (secure_mode()) {
        Status = security_policy_uninstall();
        if (Status != EFI_SUCCESS) {
            Success = FALSE;
            BeginTextScreen(L"Secure Boot Policy Failure");
            Print(L"Failed to uninstall MOK Secure Boot extensions; forcing a reboot.");
            PauseForKey();
            refit_call4_wrapper(RT->ResetSystem, EfiResetCold, EFI_SUCCESS, 0, NULL);
        }
    }
    return Success;
} // VOID SecureBootUninstall

// Sets the global configuration filename; will be CONFIG_FILE_NAME unless the
// "-c" command-line option is set, in which case that takes precedence.
// If an error is encountered, leaves the value alone (it should be set to
// CONFIG_FILE_NAME when GlobalConfig is initialized).
static VOID SetConfigFilename(EFI_HANDLE ImageHandle) {
    EFI_LOADED_IMAGE *Info;
    CHAR16 *Options, *FileName, *SubString;
    EFI_STATUS Status;

    Status = refit_call3_wrapper(BS->HandleProtocol, ImageHandle, &LoadedImageProtocol, (VOID **) &Info);
    if ((Status == EFI_SUCCESS) && (Info->LoadOptionsSize > 0)) {
        Options = (CHAR16 *) Info->LoadOptions;
        SubString = MyStrStr(Options, L" -c ");
        if (SubString) {
            FileName = StrDuplicate(&SubString[4]);
            if (FileName) {
                LimitStringLength(FileName, 256);
            }

            if (FileExists(SelfDir, FileName)) {
                GlobalConfig.ConfigFilename = FileName;
            } else {
                Print(L"Specified configuration file (%s) doesn't exist; using default\n", FileName);
                MyFreePool(FileName);
            } // if/else
        } // if
    } // if
} // VOID SetConfigFilename()

// Detect the global configuration filename; for initial compatibility with refind
static VOID DetectConfigFilename() {
    if (FileExists(SelfDir, CONFIG_FILE_NAME)) {
        LOG(3, LOG_LINE_NORMAL, L"Default configuration file (%s) found", CONFIG_FILE_NAME);
        GlobalConfig.ConfigFilename = CONFIG_FILE_NAME;
        return;
    } else {
        LOG(2, LOG_LINE_NORMAL, L"Default configuration file (%s) doesn't exist", CONFIG_FILE_NAME);
    }

    if (FileExists(SelfDir, CONFIG_FILE_NAME_COMPAT)) {
        LOG(3, LOG_LINE_NORMAL, L"Default configuration file (%s) found", CONFIG_FILE_NAME_COMPAT);
        GlobalConfig.ConfigFilename = CONFIG_FILE_NAME_COMPAT;
        return;
    } else {
        LOG(2, LOG_LINE_NORMAL, L"Compat configuration file (%s) doesn't exist", CONFIG_FILE_NAME_COMPAT);
    }

    LOG(2, LOG_LINE_NORMAL, L"No configuration file found, that will be fun");
}

// Adjust the GlobalConfig.DefaultSelection variable: Replace all "+" elements with the
// rEFIng PreviousBoot variable, if it's available. If it's not available, delete that
// element.
static VOID AdjustDefaultSelection() {
    UINTN i = 0, j;
    CHAR16 *Element = NULL, *NewCommaDelimited = NULL, *PreviousBoot = NULL;
    EFI_STATUS Status;

    LOG(3, LOG_LINE_NORMAL, L"Adjusting default_selection with PreviousBoot values");
    if (GlobalConfig.DefaultSelection)
        LOG(4, LOG_LINE_NORMAL, L"DefaultSelection[OUT]: %s", GlobalConfig.DefaultSelection);
    else
        LOG(4, LOG_LINE_NORMAL, L"DefaultSelection[OUT]: NULL");
    while ((Element = FindCommaDelimited(GlobalConfig.DefaultSelection, i++)) != NULL) {
        if (MyStriCmp(Element, L"+")) {
            Status = EfivarGetRaw(&RefindGuid, L"PreviousBoot", (CHAR8 **) &PreviousBoot, &j);
            if (Status == EFI_SUCCESS) {
                MyFreePool(Element);
                Element = PreviousBoot;
                LOG(3, LOG_LINE_NORMAL, L"PreviousBoot was '%s'", Element);
            } else {
                Element = NULL;
                LOG(3, LOG_LINE_NORMAL, L"PreviousBoot was not set");
            }
        } // if
        if (Element && StrLen(Element)) {
            MergeStrings(&NewCommaDelimited, Element, L',');
        } // if
        MyFreePool(Element);
    } // while
    MyFreePool(GlobalConfig.DefaultSelection);
    GlobalConfig.DefaultSelection = NewCommaDelimited;
    if (GlobalConfig.DefaultSelection)
        LOG(4, LOG_LINE_NORMAL, L"DefaultSelection[OUT]: %s", GlobalConfig.DefaultSelection);
    else
        LOG(4, LOG_LINE_NORMAL, L"DefaultSelection[OUT]: NULL");
} // AdjustDefaultSelection()

// Log basic information (rEFIng version, EFI version, etc.) to the log file.
VOID LogBasicInfo(VOID) {
    EFI_STATUS Status;
    UINT64     MaximumVariableStorageSize;
    UINT64     RemainingVariableStorageSize;
    UINT64     MaximumVariableSize;
    UINTN      EfiMajorVersion = ST->Hdr.Revision >> 16;
    CHAR16     *TempStr;
    EFI_GUID   ConsoleControlProtocolGuid = EFI_CONSOLE_CONTROL_PROTOCOL_GUID;
    EFI_GUID   UgaDrawProtocolGuid = EFI_UGA_DRAW_PROTOCOL_GUID;
    EFI_GUID   GraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

    LOG(3, LOG_LINE_SEPARATOR, L"System information");
    LOG(3, LOG_LINE_NORMAL, L"rEFIng %s built with GNU-EFI", REFING_VERSION);
    TempStr = GuidAsString(&(SelfVolume->PartGuid));
    LOG(3, LOG_LINE_NORMAL, L"rEFIng boot partition GUID: %s", TempStr);
    MyFreePool(TempStr);
#if defined(EFI32)
    LOG(3, LOG_LINE_NORMAL, L"Platform: x86/IA32/i386 (32-bit)");
#elif defined(EFIX64)
    LOG(3, LOG_LINE_NORMAL, L"Platform: x86-64/X64/AMD64 (64-bit)");
#elif defined(EFIAARCH64)
    LOG(3, LOG_LINE_NORMAL, L"Platform: ARM64/AARCH64 (64-bit)");
#else
    LOG(3, LOG_LINE_NORMAL, L"Platform: unknown");
#endif
    LOG(3, LOG_LINE_NORMAL, L"Log level is %d", GlobalConfig.LogLevel);
    LOG(3, LOG_LINE_NORMAL, L"Menu timeout is %d", GlobalConfig.Timeout);
    LOG(3, LOG_LINE_NORMAL, L"Firmware: %s %d.%02d", ST->FirmwareVendor,
        ST->FirmwareRevision >> 16, ST->FirmwareRevision & ((1 << 16) - 1));
    LOG(3, LOG_LINE_NORMAL, L"EFI Revision %d.%02d", EfiMajorVersion,
        ST->Hdr.Revision & ((1 << 16) - 1));
    LOG(3, LOG_LINE_NORMAL, L"Secure Boot %s", secure_mode() ? L"active" : L"inactive");
    LOG(3, LOG_LINE_NORMAL, L"Shim is%s available", ShimLoaded() ? L"" : L" not");
    switch (GlobalConfig.LegacyType) {
        case LEGACY_TYPE_MAC:
            TempStr = L"CSM type: Mac";
            break;
        case LEGACY_TYPE_UEFI:
            TempStr = L"CSM type: UEFI";
            break;
        case LEGACY_TYPE_NONE:
            TempStr = L"CSM is not available";
            break;
        default: // should never happen; just in case....
            TempStr = L"CSM type: unknown";
            break;
    }
    LOG(3, LOG_LINE_NORMAL, TempStr);
    if (EfiMajorVersion > 1) { // QueryVariableInfo() is not supported in EFI 1.x
        LOG(3, LOG_LINE_NORMAL, L"Trying to get variable info....");
        Status = refit_call4_wrapper(RT->QueryVariableInfo, EFI_VARIABLE_NON_VOLATILE,
                                     &MaximumVariableStorageSize, &RemainingVariableStorageSize,
                                     &MaximumVariableSize);
        if (EFI_ERROR(Status)) {
            LOG(2, LOG_LINE_NORMAL, L"Error %d; Unable to retrieve EFI variable capacity", Status);
        } else {
            LOG(3, LOG_LINE_NORMAL, L"EFI non-volatile storage:");
            LOG(3, LOG_LINE_NORMAL, L"   Total storage: %ld", MaximumVariableStorageSize);
            LOG(3, LOG_LINE_NORMAL, L"   Remaining available: %ld", RemainingVariableStorageSize);
            LOG(3, LOG_LINE_NORMAL, L"   Maximum variable size: %ld", MaximumVariableSize);
        }
    } else {
        LOG(2, LOG_LINE_NORMAL, L"EFI 1.x; EFI non-volatile storage information is unavailable");
    }

    // Report which video output devices are available. We don't actually
    // use them, so just use TempStr as a throwaway pointer to the protocol.
    Status = LibLocateProtocol(&ConsoleControlProtocolGuid, (VOID **) &TempStr);
    LOG(3, LOG_LINE_NORMAL, L"System does%s support text mode",
        EFI_ERROR(Status) ? L" not" : L"");

    Status = LibLocateProtocol(&UgaDrawProtocolGuid, (VOID **) &TempStr);
    LOG(3, LOG_LINE_NORMAL, L"System does%s support UGA Draw graphics mode",
        EFI_ERROR(Status) ? L" not" : L"");

    Status = LibLocateProtocol(&GraphicsOutputProtocolGuid, (VOID **) &TempStr);
    LOG(3, LOG_LINE_NORMAL, L"System does%s support GOP graphics mode",
        EFI_ERROR(Status) ? L" not" : L"");
} // VOID LogBasicInfo()

//
// main entry point
//
EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS         Status;
    BOOLEAN            MainLoopRunning = TRUE;
    BOOLEAN            MokProtocol;
    REFIT_MENU_ENTRY   *ChosenEntry;
    UINTN              MenuExit = MENU_EXIT_ENTER, i;
    CHAR16             *SelectionName = NULL;
    EG_PIXEL           BGColor = COLOR_LIGHTBLUE;

    // bootstrap
    InitializeLib(ImageHandle, SystemTable);
    TimeInit(ImageHandle);

    // initialize logging, to buffer until LOG_Activate
    LOG_Init();

    LOG(4, LOG_LINE_SEPARATOR, L"Log start");
    LOG(4, LOG_LINE_NORMAL, L"Initializing");

    // Initialize lib
    Status = InitRefitLib(ImageHandle);
    if (EFI_ERROR(Status))
        return Status;

    // read configuration
    GlobalConfig.ScanFor = SCANFOR_FLAG_INT |
                           SCANFOR_FLAG_EXT |
                           SCANFOR_FLAG_OPTICAL |
                           SCANFOR_FLAG_MANUAL;
    FindLegacyBootType();
    if (GlobalConfig.LegacyType == LEGACY_TYPE_MAC)
       GlobalConfig.ScanFor |= SCANFOR_FLAG_LEGACY;
    SetConfigFilename(ImageHandle);
    if (!GlobalConfig.ConfigFilename)
       DetectConfigFilename();

    // Scan volumes first to find SelfVolume, which is needed by LoadDrivers()
    // and ReadConfig(); however, if drivers are loaded, a second call to
    // ScanVolumes() is needed to register the new filesystem(s) accessed
    // by the drivers.
    ScanVolumes();
    ReadConfig(GlobalConfig.ConfigFilename);

    LOG_Activate(GlobalConfig.LogDirect);
    if (GlobalConfig.LogLevel > 0)
        LogBasicInfo();

    InitScreen();

    LOG(3, LOG_LINE_NORMAL, L"GlobalConfig.DontScanFiles is '%s'", GlobalConfig.DontScanFiles);
    MokProtocol = SecureBootSetup();
    if (LoadDrivers())
        ScanVolumes();

    LOG(3, LOG_LINE_SEPARATOR, L"Initializing basic features");
    AdjustDefaultSelection();

    if (GlobalConfig.SpoofOSXVersion && GlobalConfig.SpoofOSXVersion[0] != L'\0')
        SetAppleOSInfo();

    WarnIfLegacyProblems();
    MainMenu.TimeoutSeconds = GlobalConfig.Timeout;

    // disable EFI watchdog timer
    LOG(3, LOG_LINE_NORMAL, L"Setting watchdog timer");
    refit_call4_wrapper(BS->SetWatchdogTimer, 0x0000, 0x0000, 0x0000, NULL);

    // further bootstrap (now with config available)
    SetupScreen();
    SetVolumeIcons();
    ScanForBootloaders(FALSE);
    ScanForTools();

    // SetupScreen() clears the screen; but ScanForBootloaders() may display a
    // message that must be deleted, so do so
    BltBackgroundScreen();
//    SetupScreen();
    pdInitialize();

    if (GlobalConfig.ScanDelay > 0) {
       if (GlobalConfig.ScanDelay > 1) {
          LOG(3, LOG_LINE_NORMAL, L"Pausing before re-scan");
          egDisplayMessage(L"Pausing before disk scan; please wait....", &BGColor, CENTER);
       }
       for (i = 0; i < GlobalConfig.ScanDelay; i++)
          refit_call1_wrapper(BS->Stall, 1000000);
       RescanAll(GlobalConfig.ScanDelay > 1, TRUE);
       BltBackgroundScreen();
    } // if

    if (GlobalConfig.DefaultSelection)
       SelectionName = StrDuplicate(GlobalConfig.DefaultSelection);
    if (GlobalConfig.ShutdownAfterTimeout)
        MainMenu.TimeoutText = L"Shutdown";

    LOG(3, LOG_LINE_SEPARATOR, L"Entering main loop");
    while (MainLoopRunning) {
        MenuExit = RunMainMenu(&MainMenu, &SelectionName, &ChosenEntry);

        // The Escape key triggers a re-scan operation....
        if (MenuExit == MENU_EXIT_ESCAPE) {
            MenuExit = 0;
            RescanAll(TRUE, TRUE);
            continue;
        }

        if ((MenuExit == MENU_EXIT_TIMEOUT) && GlobalConfig.ShutdownAfterTimeout) {
            ChosenEntry->Tag = TAG_SHUTDOWN;
        }

        switch (ChosenEntry->Tag) {

            case TAG_REBOOT:    // Reboot
                TerminateScreen();
                LOG(3, LOG_LINE_SEPARATOR, L"Rebooting system");
                refit_call4_wrapper(RT->ResetSystem, EfiResetCold, EFI_SUCCESS, 0, NULL);
                LOG(1, LOG_LINE_NORMAL, L"Reboot FAILED!");
                MainLoopRunning = FALSE;   // just in case we get this far
                break;

            case TAG_SHUTDOWN: // Shut Down
                TerminateScreen();
                LOG(3, LOG_LINE_SEPARATOR, L"Shutting down system");
                refit_call4_wrapper(RT->ResetSystem, EfiResetShutdown, EFI_SUCCESS, 0, NULL);
                LOG(1, LOG_LINE_NORMAL, L"Shutdown FAILED!");
                MainLoopRunning = FALSE;   // just in case we get this far
                break;

            case TAG_ABOUT:    // About rEFIng
                AboutrEFIng();
                break;

            case TAG_LOADER:   // Boot OS via .EFI loader
                StartLoader((LOADER_ENTRY *)ChosenEntry, SelectionName);
                break;

            case TAG_LEGACY:   // Boot legacy OS
                StartLegacy((LEGACY_ENTRY *)ChosenEntry, SelectionName);
                break;

            case TAG_LEGACY_UEFI: // Boot a legacy OS on a non-Mac
                StartLegacyUEFI((LEGACY_ENTRY *)ChosenEntry, SelectionName);
                break;

            case TAG_FIRMWARE_LOADER: // Reboot to a loader defined in the EFI UseNVRAM
                RebootIntoLoader((LOADER_ENTRY *)ChosenEntry);
                break;

            case TAG_TOOL:     // Start a EFI tool
                StartTool((LOADER_ENTRY *)ChosenEntry);
                break;

            case TAG_HIDDEN:  // Manage hidden tag entries
                ManageHiddenTags();
                break;

            case TAG_EXIT:    // Terminate rEFIng
                if ((MokProtocol) && !SecureBootUninstall()) {
                   MainLoopRunning = FALSE;   // just in case we get this far
                } else {
                   BeginTextScreen(L" ");
                   return EFI_SUCCESS;
                }
                break;

            case TAG_FIRMWARE: // Reboot into firmware's user interface
                RebootIntoFirmware();
                break;

            case TAG_CSR_ROTATE:
                RotateCsrValue();
                break;

            case TAG_INSTALL:
                InstallRefing();
                break;

            case TAG_BOOTORDER:
                ManageBootorder();
                break;

        } // switch()
    } // while()
    MyFreePool(SelectionName);

    // If we end up here, things have gone wrong. Try to reboot, and if that
    // fails, go into an endless loop.
    LOG(1, LOG_LINE_SEPARATOR, L"Main loop has exited, but it should not have!");
    UninitRefitLib();
    refit_call4_wrapper(RT->ResetSystem, EfiResetCold, EFI_SUCCESS, 0, NULL);
    ReinitRefitLib();
    LOG(1, LOG_LINE_SEPARATOR, L"Shutdown after main loop exit has FAILED!");
    LOG_End();
    EndlessIdleLoop();

    return EFI_SUCCESS;
} /* efi_main() */
