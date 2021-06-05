/*
 * BootMaster/main.c
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
 * Modifications copyright (c) 2012-2021 Roderick W. Smith
 *
 * Modifications distributed under the terms of the GNU General Public
 * License (GPL) version 3 (GPLv3), or (at your option) any later version.
 */
/*
 * Modified for RefindPlus
 * Copyright (c) 2020-2021 Dayo Akanji (sf.net/u/dakanji/profile)
 *
 * Modifications distributed under the preceding terms.
 */

#include "global.h"
#include "config.h"
#include "screenmgt.h"
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
#include "../include/refit_call_wrapper.h"
#include "../libeg/efiConsoleControl.h"
#include "../libeg/efiUgaDraw.h"
#include "../include/version.h"
#include "../libeg/libeg.h"

#ifndef __MAKEWITH_GNUEFI
#define LibLocateProtocol EfiLibLocateProtocol
#endif

INT16 NowYear   = 0;
INT16 NowMonth  = 0;
INT16 NowDay    = 0;
INT16 NowHour   = 0;
INT16 NowMinute = 0;
INT16 NowSecond = 0;

//
// Some built-in menu definitions....

REFIT_MENU_ENTRY MenuEntryReturn = {
    L"Return to Main Menu",
    TAG_RETURN,
    1, 0, 0,
    NULL, NULL, NULL
};

REFIT_MENU_SCREEN MainMenu = {
    L"Main Menu",
    NULL,
    0, NULL, 0,
    NULL, 0, L"Automatic boot",
    L"Use arrow keys to move cursor; Enter to boot;",
    L"Insert, Tab, or F2 for more options; Esc or Backspace to refresh"
};

REFIT_MENU_SCREEN AboutMenu = {
    L"About RefindPlus",
    NULL,
    0, NULL, 0,
    NULL, 0, NULL,
    L"Press 'Enter' to return to main menu",
    L""
};

REFIT_MENU_ENTRY MenuEntryBootKicker = {
    L"Load BootKicker",
    TAG_SHOW_BOOTKICKER,
    1, 0, 0,
    NULL, NULL, NULL
};

REFIT_MENU_ENTRY MenuEntryCleanNvram = {
    L"Load CleanNvram",
    TAG_NVRAMCLEAN,
    1, 0, 0,
    NULL, NULL, NULL
};

REFIT_CONFIG GlobalConfig = {
    /* TextOnly = */ FALSE,
    /* ScanAllLinux = */ TRUE,
    /* DeepLegacyScan = */ FALSE,
    /* EnableAndLockVMX = */ FALSE,
    /* FoldLinuxKernels = */ TRUE,
    /* EnableMouse = */ FALSE,
    /* EnableTouch = */ FALSE,
    /* HiddenTags = */ TRUE,
    /* UseNvram = */ FALSE,
    /* IgnorePreviousBoot = */ FALSE,
    /* IgnoreVolumeICNS = */ FALSE,
    /* TextRenderer = */ FALSE,
    /* UgaPassThrough = */ FALSE,
    /* ProvideConsoleGOP = */ FALSE,
    /* ReloadGOP = */ FALSE,
    /* UseDirectGop = */ FALSE,
    /* ContinueOnWarning = */ FALSE,
    /* ForceTRIM = */ FALSE,
    /* DisableCompatCheck = */ FALSE,
    /* DisableAMFI = */ FALSE,
    /* SupplyAPFS = */ FALSE,
    /* SilenceAPFS = */ FALSE,
    /* SyncAPFS = */ FALSE,
    /* ProtectNVRAM = */ FALSE,
    /* ScanOtherESP = */ FALSE,
    /* NormaliseCSR = */ FALSE,
    /* ShutdownAfterTimeout = */ FALSE,
    /* Install = */ FALSE,
    /* WriteSystemdVars = */ FALSE,
    /* RequestedScreenWidth = */ 0,
    /* RequestedScreenHeight = */ 0,
    /* BannerBottomEdge = */ 0,
    /* RequestedTextMode = */ DONT_CHANGE_TEXT_MODE,
    /* Timeout = */ 0,
    /* HideUIFlags = */ 0,
    /* MaxTags = */ 0,
    /* GraphicsFor = */ GRAPHICS_FOR_OSX,
    /* LegacyType = */ LEGACY_TYPE_MAC,
    /* ScanDelay = */ 0,
    /* ScreensaverTime = */ 0,
    /* MouseSpeed = */ 4,
    /* IconSizes = */ {
        DEFAULT_BIG_ICON_SIZE / 4,
        DEFAULT_SMALL_ICON_SIZE,
        DEFAULT_BIG_ICON_SIZE,
        DEFAULT_MOUSE_SIZE
    },
    /* BannerScale = */ BANNER_NOSCALE,
    /* ScaleUI = */ 0,
    /* ActiveCSR = */ 0,
    /* LogLevel = */ 0,
    /* *DiscoveredRoot = */ NULL,
    /* *SelfDevicePath = */ NULL,
    /* *BannerFileName = */ NULL,
    /* *ScreenBackground = */ NULL,
    /* *ConfigFilename = */ CONFIG_FILE_NAME,
    /* *SelectionSmallFileName = */ NULL,
    /* *SelectionBigFileName = */ NULL,
    /* *DefaultSelection = */ NULL,
    /* *AlsoScan = */ NULL,
    /* *DontScanVolumes = */ NULL,
    /* *DontScanDirs = */ NULL,
    /* *DontScanFiles = */ NULL,
    /* *DontScanTools = */ NULL,
    /* *DontScanFirmware = */ NULL,
    /* *WindowsRecoveryFiles = */ NULL,
    /* *MacOSRecoveryFiles = */ NULL,
    /* *DriverDirs = */ NULL,
    /* *IconsDir = */ NULL,
    /* *SetBootArgs = */ NULL,
    /* *ExtraKernelVersionStrings = */ NULL,
    /* *SpoofOSXVersion = */ NULL,
    /* CsrValues = */ NULL,
    /* ShowTools = */ {
        TAG_SHELL,
        TAG_MEMTEST,
        TAG_GDISK,
        TAG_APPLE_RECOVERY,
        TAG_WINDOWS_RECOVERY,
        TAG_MOK_TOOL,
        TAG_ABOUT,
        TAG_HIDDEN,
        TAG_SHUTDOWN,
        TAG_REBOOT,
        TAG_FIRMWARE,
        TAG_FWUPDATE_TOOL,
        0, 0, 0, 0, 0, 0, 0, 0, 0
    }
};

#define BOOTKICKER_FILES L"\\EFI\\tools_x64\\x64_BootKicker.efi,\\EFI\\tools_x64\\BootKicker_x64.efi,\
\\EFI\\tools_x64\\BootKicker.efi,\\EFI\\tools\\x64_BootKicker.efi,\\EFI\\tools\\BootKicker_x64.efi,\
\\EFI\\tools\\BootKicker.efi,\\EFI\\x64_BootKicker.efi,\\EFI\\BootKicker_x64.efi,\\EFI\\BootKicker.efi,\
\\x64_BootKicker.efi,\\BootKicker_x64.efi,\\BootKicker.efi"

#define NVRAMCLEAN_FILES L"\\EFI\\tools_x64\\x64_CleanNvram.efi,\\EFI\\tools_x64\\CleanNvram_x64.efi,\
\\EFI\\tools_x64\\CleanNvram.efi,\\EFI\\tools\\x64_CleanNvram.efi,\\EFI\\tools\\CleanNvram_x64.efi,\
\\EFI\\tools\\CleanNvram.efi,\\EFI\\x64_CleanNvram.efi,\\EFI\\CleanNvram_x64.efi,\\EFI\\CleanNvram.efi,\
\\x64_CleanNvram.efi,\\CleanNvram_x64.efi,\\CleanNvram.efi"

CHAR16                *VendorInfo           = NULL;
CHAR16                *gHiddenTools         = NULL;
BOOLEAN                SetSysTab            = FALSE;
BOOLEAN                ConfigWarn           = FALSE;
BOOLEAN                ranCleanNvram        = FALSE;
BOOLEAN                ForceNativeLoggging  = FALSE;
EFI_GUID               RefindPlusGuid       = REFINDPLUS_GUID;
EFI_SET_VARIABLE       AltSetVariable;
EFI_OPEN_PROTOCOL      OrigOpenProtocol;
EFI_HANDLE_PROTOCOL    OrigHandleProtocol;

#if REFIT_DEBUG > 0
extern VOID InitBooterLog (VOID);
#endif

extern EFI_STATUS RpApfsConnectDevices (VOID);

// Link to Cert GUIDs in mok/guid.c
extern EFI_GUID X509_GUID;
extern EFI_GUID RSA2048_GUID;
extern EFI_GUID PKCS7_GUID;
extern EFI_GUID EFI_CERT_SHA256_GUID;

extern EFI_FILE *gVarsDir;

extern EFI_GRAPHICS_OUTPUT_PROTOCOL *GOPDraw;

//
// misc functions
//

static
EFI_STATUS EFIAPI gRTSetVariableEx (
    IN  CHAR16    *VariableName,
    IN  EFI_GUID  *VendorGuid,
    IN  UINT32     Attributes,
    IN  UINTN      VariableSize,
    IN  VOID      *VariableData
) {
    EFI_STATUS   Status                 = EFI_SECURITY_VIOLATION;
    EFI_GUID     WinGuid                = MICROSOFT_VENDOR_GUID;
    EFI_GUID     X509Guid               = X509_GUID;
    EFI_GUID     PKCS7Guid              = PKCS7_GUID;
    EFI_GUID     Sha001Guid             = EFI_CERT_SHA1_GUID;
    EFI_GUID     Sha224Guid             = EFI_CERT_SHA224_GUID;
    EFI_GUID     Sha256Guid             = EFI_CERT_SHA256_GUID;
    EFI_GUID     Sha384Guid             = EFI_CERT_SHA384_GUID;
    EFI_GUID     Sha512Guid             = EFI_CERT_SHA512_GUID;
    EFI_GUID     RSA2048Guid            = RSA2048_GUID;
    EFI_GUID     RSA2048Sha1Guid        = EFI_CERT_RSA2048_SHA1_GUID;
    EFI_GUID     RSA2048Sha256Guid      = EFI_CERT_RSA2048_SHA256_GUID;
    EFI_GUID     TypeRSA2048Sha256Guid  = EFI_CERT_TYPE_RSA2048_SHA256_GUID;
    UINT32       StorageFlags;

    #if REFIT_DEBUG > 0
    CHAR16 *MsgStr = NULL;
    #endif

    BOOLEAN BlockCert = (
        (GuidsAreEqual (VendorGuid, &WinGuid) ||
        (GuidsAreEqual (VendorGuid, &X509Guid)) ||
        (GuidsAreEqual (VendorGuid, &PKCS7Guid)) ||
        (GuidsAreEqual (VendorGuid, &Sha001Guid)) ||
        (GuidsAreEqual (VendorGuid, &Sha224Guid)) ||
        (GuidsAreEqual (VendorGuid, &Sha256Guid)) ||
        (GuidsAreEqual (VendorGuid, &Sha384Guid)) ||
        (GuidsAreEqual (VendorGuid, &Sha512Guid)) ||
        (GuidsAreEqual (VendorGuid, &RSA2048Guid)) ||
        (GuidsAreEqual (VendorGuid, &RSA2048Sha1Guid)) ||
        (GuidsAreEqual (VendorGuid, &RSA2048Sha256Guid)) ||
        (GuidsAreEqual (VendorGuid, &TypeRSA2048Sha256Guid))) &&
        (MyStrStr (VendorInfo, L"Apple") != NULL)
    );
    BOOLEAN BlockPRNG = (
        (MyStriCmp (VariableName, L"UnlockID") || MyStriCmp (VariableName, L"UnlockIDCopy")) &&
        MyStrStr (VendorInfo, L"Apple") != NULL
    );

    if (!BlockCert && !BlockPRNG) {
        StorageFlags  = EFI_VARIABLE_BOOTSERVICE_ACCESS;
        StorageFlags |= EFI_VARIABLE_RUNTIME_ACCESS;
        StorageFlags |= EFI_VARIABLE_NON_VOLATILE;
        Status = AltSetVariable (
            VariableName,
            VendorGuid,
            StorageFlags,
            VariableSize,
            (CHAR8 *) &VariableData
        );
    }


    #if REFIT_DEBUG > 0
    MsgStr = PoolPrint (L"Filtered Write to NVRAM:- '%s' ... %r", VariableName, Status);
    LOG(3, LOG_LINE_NORMAL, L"%s", MsgStr);
    MsgLog ("INFO: %s", MsgStr);
    MyFreePool (&MsgStr);

    if (BlockCert || BlockPRNG) {
        MsgStr = StrDuplicate (L"Prevented Microsoft Secure Boot NVRAM Write Attempt");
        LOG(3, LOG_THREE_STAR_MID, L"%s", MsgStr);
        MsgLog ("\n");
        MsgLog ("      * %s", MsgStr);
        MyFreePool (&MsgStr);

        MsgLog ("\n");
        MsgLog ("        Successful NVRAM Write May Result in BootROM Damage");
    }
    MsgLog ("\n\n");
    #endif

    return Status;
} // VOID gRTSetVariableEx()

static
VOID MapSetVariable (
    IN EFI_SYSTEM_TABLE  *SystemTable
) {
    AltSetVariable                             = gRT->SetVariable;
    RT->SetVariable                            = gRTSetVariableEx;
    gRT->SetVariable                           = gRTSetVariableEx;
    SystemTable->RuntimeServices->SetVariable  = gRTSetVariableEx;
} // static VOID MapSetVariable()

static
VOID FilterCSR (VOID) {
    if (GlobalConfig.NormaliseCSR) {
        // Filter out the 'APPLE_INTERNAL' CSR bit
        BOOLEAN FilterCSR = NormaliseCSR();

        if (FilterCSR) {
            #if REFIT_DEBUG > 0
            CHAR16 *MsgStr = StrDuplicate (L"Normalised CSR");
            LOG(3, LOG_LINE_NORMAL, L"%s", MsgStr);
            MsgLog ("\n");
            MsgLog ("    * %s", MsgStr);
            MyFreePool (&MsgStr);
            #endif
        }
    }
} // static VOID FilterCSR()

static
VOID ActiveCSR (
    VOID
) {
    UINT32  CsrStatus;
    BOOLEAN CsrEnabled = FALSE;
    BOOLEAN RotateCsr  = FALSE;

    // Prime 'Status' for logging
    #if REFIT_DEBUG > 0
    EFI_STATUS Status = EFI_ALREADY_STARTED;
    #endif

    if (GlobalConfig.ActiveCSR == 0) {
        // Early return if not configured to set CSR
        return;
    }
    else {
        // Try to get current CSR status
        if (GetCsrStatus (&CsrStatus) == EFI_SUCCESS) {
            // Record CSR status in the 'gCsrStatus' variable
            RecordgCsrStatus (CsrStatus, FALSE);

            // Check 'gCsrStatus' variable for 'Enabled' term
            if (MyStrStr (gCsrStatus, L"Enabled") != NULL) {
                // 'Enabled' found
                CsrEnabled = TRUE;
            }
            else {
                // 'Enabled' not found
                CsrEnabled = FALSE;
            }

            // If set to always disable
            if (GlobalConfig.ActiveCSR == -1) {
                // Seed the log buffer
                #if REFIT_DEBUG > 0
                MsgLog ("INFO: Disable SIP/SSV ...");
                #endif

                if (CsrEnabled) {
                    // Switch SIP/SSV off as currently enabled
                    RotateCsr = TRUE;
                }
            }
            else if (GlobalConfig.ActiveCSR == 1) {
                // Seed the log buffer
                #if REFIT_DEBUG > 0
                MsgLog ("INFO: Enable SIP/SSV ...");
                #endif

                if (!CsrEnabled) {
                    // Switch SIP/SSV on as currently disbled
                    RotateCsr = TRUE;
                }
            }
            else {
                // Should never get here
                return;
            }

            if (RotateCsr) {
                // Switch SIP/SSV off as currently enabled
                RotateCsrValue ();

                // Set 'Status' to 'Success'
                #if REFIT_DEBUG > 0
                Status = EFI_SUCCESS;
                #endif
            }

            // Finalise and flush the log buffer
            #if REFIT_DEBUG > 0
            MsgLog ("%r\n\n", Status);
            #endif
        }
    }
} // static VOID ActiveCSR()


static
VOID SetBootArgs (
    VOID
) {
    EFI_STATUS   Status;
    EFI_GUID     AppleGUID  = APPLE_GUID;
    CHAR16      *NameNVRAM  = L"boot-args";
    CHAR16      *BootArg;
    CHAR8        DataNVRAM[255];

    #if REFIT_DEBUG > 0
    CHAR16  *MsgStr                = NULL;
    BOOLEAN  LogDisableAMFI        = FALSE;
    BOOLEAN  LogDisableCompatCheck = FALSE;
    #endif

    if (!GlobalConfig.SetBootArgs || GlobalConfig.SetBootArgs[0] == L'\0') {
        #if REFIT_DEBUG > 0
        Status = EFI_INVALID_PARAMETER;
        #endif
    }
    else {
        if (MyStrStr (GlobalConfig.SetBootArgs, L"amfi_get_out_of_my_way=1") != NULL) {
            #if REFIT_DEBUG > 0
            if (GlobalConfig.DisableAMFI) {
                // Ensure Logging
                LogDisableAMFI = TRUE;
            }
            #endif

            // Do not duplicate 'amfi_get_out_of_my_way=1'
            GlobalConfig.DisableAMFI = FALSE;
        }
        if (MyStrStr (GlobalConfig.SetBootArgs, L"-no_compat_check") != NULL) {
            #if REFIT_DEBUG > 0
            if (GlobalConfig.DisableCompatCheck) {
                // Ensure Logging
                LogDisableCompatCheck = TRUE;
            }
            #endif

            // Do not duplicate '-no_compat_check'
            GlobalConfig.DisableCompatCheck = FALSE;
        }

        if (GlobalConfig.DisableAMFI &&
            GlobalConfig.DisableCompatCheck
        ) {
            // Combine Args with DisableAMFI and DisableAMFI
            BootArg = PoolPrint (
                L"%s amfi_get_out_of_my_way=1 -no_compat_check",
                GlobalConfig.SetBootArgs
            );
        }
        else if (GlobalConfig.DisableAMFI) {
            // Combine Args with DisableAMFI
            BootArg = PoolPrint (
                L"%s amfi_get_out_of_my_way=1",
                GlobalConfig.SetBootArgs
            );
        }
        else if (GlobalConfig.DisableCompatCheck) {
            // Combine Args with DisableCompatCheck
            BootArg = PoolPrint (
                L"%s -no_compat_check",
                GlobalConfig.SetBootArgs
            );
        }
        else {
            // Use Args Alone
            BootArg = PoolPrint (L"%s", GlobalConfig.SetBootArgs);
        }

        // Convert BootArg to CHAR8 array in 'ArrCHAR8'
        MyUnicodeStrToAsciiStr  (BootArg, DataNVRAM);
        MyFreePool (&BootArg);

        Status = EfivarSetRaw (
            &AppleGUID,
            NameNVRAM,
            DataNVRAM,
            sizeof (DataNVRAM),
            TRUE
        );
    }

    #if REFIT_DEBUG > 0
    if (LogDisableAMFI || GlobalConfig.DisableAMFI) {
        MsgStr = PoolPrint (
            L"Disable AMFI ... %r",
            Status
        );
        LOG(3, LOG_LINE_NORMAL, L"%s", MsgStr);
        MsgLog ("\n");
        MsgLog ("    * %s", MsgStr);
        MyFreePool (&MsgStr);
    }

    MsgStr = PoolPrint (
        L"Reset Boot Args ... %r",
        Status
    );
    LOG(3, LOG_LINE_NORMAL, L"%s", MsgStr);
    MsgLog ("\n");
    MsgLog ("    * %s", MsgStr);
    MyFreePool (&MsgStr);

    if (LogDisableCompatCheck || GlobalConfig.DisableCompatCheck) {
        MsgStr = PoolPrint (
            L"Disable Compat Check ... %r",
            Status
        );
        LOG(3, LOG_LINE_NORMAL, L"%s", MsgStr);
        MsgLog ("\n");
        MsgLog ("    * %s", MsgStr);
        MyFreePool (&MsgStr);
    }
    #endif
} // static VOID SetBootArgs()


VOID DisableAMFI (
    VOID
) {
    EFI_STATUS   Status;
    EFI_GUID     AppleGUID  = APPLE_GUID;
    CHAR16      *NameNVRAM  = L"boot-args";

    #if REFIT_DEBUG > 0
    CHAR16  *MsgStr = NULL;
    #endif

    if (GlobalConfig.DisableCompatCheck) {
        // Combine with DisableCompatCheck
        CHAR8 DataNVRAM[] = "amfi_get_out_of_my_way=1 -no_compat_check";

        Status = EfivarSetRaw (
            &AppleGUID,
            NameNVRAM,
            DataNVRAM,
            sizeof (DataNVRAM),
            TRUE
        );
    }
    else {
        CHAR8 DataNVRAM[] = "amfi_get_out_of_my_way=1";

        Status = EfivarSetRaw (
            &AppleGUID,
            NameNVRAM,
            DataNVRAM,
            sizeof (DataNVRAM),
            TRUE
        );
    }

    #if REFIT_DEBUG > 0
    MsgStr = PoolPrint (
        L"Disable AMFI ... %r",
        Status
    );
    LOG(3, LOG_LINE_NORMAL, L"%s", MsgStr);
    MsgLog ("\n");
    MsgLog ("    * %s", MsgStr);
    MyFreePool (&MsgStr);

    if (GlobalConfig.DisableCompatCheck) {
        MsgStr = PoolPrint (
            L"Disable Compat Check ... %r",
            Status
        );
        LOG(3, LOG_LINE_NORMAL, L"%s", MsgStr);
        MsgLog ("\n");
        MsgLog ("    * %s", MsgStr);
        MyFreePool (&MsgStr);
    }
    #endif
} // VOID DisableAMFI()


VOID DisableCompatCheck (
    VOID
) {
    EFI_STATUS   Status;
    EFI_GUID     AppleGUID    = APPLE_GUID;
    CHAR16      *NameNVRAM    = L"boot-args";
    CHAR8        DataNVRAM[]  = "-no_compat_check";

    #if REFIT_DEBUG > 0
    CHAR16  *MsgStr = NULL;
    #endif

    Status = EfivarSetRaw (
        &AppleGUID,
        NameNVRAM,
        DataNVRAM,
        sizeof (DataNVRAM),
        TRUE
    );

    #if REFIT_DEBUG > 0
    MsgStr = PoolPrint (
        L"Disable Compat Check ... %r",
        Status
    );
    LOG(3, LOG_LINE_NORMAL, L"%s", MsgStr);
    MsgLog ("\n");
    MsgLog ("    * %s", MsgStr);
    MyFreePool (&MsgStr);
    #endif
} // VOID DisableCompatCheck()


VOID ForceTRIM (
    VOID
) {
    EFI_STATUS   Status;
    EFI_GUID     AppleGUID     = APPLE_GUID;
    CHAR16      *NameNVRAM     = L"EnableTRIM";
    CHAR8        DataNVRAM[1]  = {0x01};

    #if REFIT_DEBUG > 0
    CHAR16  *MsgStr = NULL;
    #endif

    Status = EfivarSetRaw (
        &AppleGUID,
        NameNVRAM,
        DataNVRAM,
        sizeof (DataNVRAM),
        TRUE
    );

    #if REFIT_DEBUG > 0
    MsgStr = PoolPrint (
        L"Forcibly Enable TRIM ... %r",
        Status
    );
    LOG(3, LOG_LINE_NORMAL, L"%s", MsgStr);
    MsgLog ("\n");
    MsgLog ("    * %s", MsgStr);
    MyFreePool (&MsgStr);
    #endif
} // VOID ForceTRIM()


// Extended 'OpenProtocol'
// Ensures GOP Interface for Boot Loading Screen
static
EFI_STATUS EFIAPI OpenProtocolEx (
    IN   EFI_HANDLE    Handle,
    IN   EFI_GUID     *Protocol,
    OUT  VOID        **Interface OPTIONAL,
    IN   EFI_HANDLE    AgentHandle,
    IN   EFI_HANDLE    ControllerHandle,
    IN   UINT32        Attributes
) {
    EFI_STATUS   Status;
    UINTN        i              = 0;
    UINTN        HandleCount    = 0;
    EFI_HANDLE  *HandleBuffer   = NULL;

    Status = OrigOpenProtocol (
        Handle,
        Protocol,
        Interface,
        AgentHandle,
        ControllerHandle,
        Attributes
    );

    if (Status == EFI_UNSUPPORTED) {
        if (GuidsAreEqual (&gEfiGraphicsOutputProtocolGuid, Protocol)) {
            if (GOPDraw != NULL) {
                Status     = EFI_SUCCESS;
                *Interface = GOPDraw;
            }
            else {
                Status = refit_call5_wrapper(
                    gBS->LocateHandleBuffer,
                    ByProtocol,
                    &gEfiGraphicsOutputProtocolGuid,
                    NULL,
                    &HandleCount,
                    &HandleBuffer
                );

                if (!EFI_ERROR (Status)) {
                    for (i = 0; i < HandleCount; i++) {
                        if (HandleBuffer[i] != gST->ConsoleOutHandle) {
                            Status = refit_call6_wrapper(
                                OrigOpenProtocol,
                                HandleBuffer[i],
                                &gEfiGraphicsOutputProtocolGuid,
                                *Interface,
                                AgentHandle,
                                NULL,
                                EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
                            );

                            if (!EFI_ERROR (Status)) {
                                break;
                            }
                        } // if HandleBuffer[i]
                    } // for

                } // if !EFI_ERROR Status

                if (EFI_ERROR (Status) || *Interface == NULL) {
                    Status = EFI_UNSUPPORTED;
                }
            } // If GOPDraw != NULL
        } // if GuidsAreEqual

        MyFreePool (&HandleBuffer);
    } // if Status == EFI_UNSUPPORTED

    return Status;
} // EFI_STATUS OpenProtocolEx()


// Extended 'HandleProtocol'
// Routes 'HandleProtocol' to 'OpenProtocol'
static
EFI_STATUS EFIAPI HandleProtocolEx (
    IN   EFI_HANDLE   Handle,
    IN   EFI_GUID    *Protocol,
    OUT  VOID       **Interface
) {
    EFI_STATUS Status;

    Status = refit_call6_wrapper(
        gBS->OpenProtocol,
        Handle,
        Protocol,
        Interface,
        gImageHandle,
        NULL,
        EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
    );

    return Status;
} // EFI_STATUS HandleProtocolEx()

static
VOID ReMapOpenProtocol (
    VOID
) {
    // Amend EFI_BOOT_SERVICES.OpenProtocol
    OrigOpenProtocol    = gBS->OpenProtocol;
    gBS->OpenProtocol   = OpenProtocolEx;
    gBS->Hdr.CRC32      = 0;
    gBS->CalculateCrc32 (gBS, gBS->Hdr.HeaderSize, &gBS->Hdr.CRC32);
} // ReMapOpenProtocol()


// Checks to see if a specified file seems to be a valid tool.
// Returns TRUE if it passes all tests, FALSE otherwise
static
BOOLEAN IsValidTool (
    IN  REFIT_VOLUME  *BaseVolume,
    IN  CHAR16        *PathName
) {
    UINTN     i            = 0;
    BOOLEAN   retval       = TRUE;
    CHAR16   *TestVolName  = NULL;
    CHAR16   *DontVolName  = NULL;
    CHAR16   *DontPathName = NULL;
    CHAR16   *DontFileName = NULL;
    CHAR16   *TestPathName = NULL;
    CHAR16   *TestFileName = NULL;
    CHAR16   *DontScanThis = NULL;

    if (FileExists (BaseVolume->RootDir, PathName) &&
        IsValidLoader (BaseVolume->RootDir, PathName)
    ) {
        SplitPathName (PathName, &TestVolName, &TestPathName, &TestFileName);

        while (retval && (DontScanThis = FindCommaDelimited (GlobalConfig.DontScanTools, i++))) {
            SplitPathName (DontScanThis, &DontVolName, &DontPathName, &DontFileName);

            if (MyStriCmp (TestFileName, DontFileName) &&
                ((DontPathName == NULL) || (MyStriCmp (TestPathName, DontPathName))) &&
                ((DontVolName == NULL) || (VolumeMatchesDescription (BaseVolume, DontVolName)))
            ) {
                retval = FALSE;
            } // if

            MyFreePool (&DontScanThis);
        } // while

    }
    else {
        retval = FALSE;
    }

    MyFreePool (&TestVolName);
    MyFreePool (&TestPathName);
    MyFreePool (&TestFileName);

    return retval;
} // BOOLEAN IsValidTool()

VOID preBootKicker (
    VOID
) {
    UINTN              MenuExit;
    INTN               DefaultEntry   = 1;
    MENU_STYLE_FUNC    Style          = GraphicsMenuStyle;
    REFIT_MENU_ENTRY  *ChosenEntry;
    REFIT_MENU_ENTRY  *TempMenuEntry  = CopyMenuEntry (&MenuEntryBootKicker);
    TempMenuEntry->Image              = BuiltinIcon (BUILTIN_ICON_TOOL_BOOTKICKER);
    CHAR16            *MenuInfo       = L"A tool to kick in the Apple Boot Screen";
    REFIT_MENU_SCREEN  BootKickerMenu = {
        L"BootKicker",
        NULL, 0, &MenuInfo,
        0, &TempMenuEntry,
        0, NULL,
        L"Press 'ESC', 'BackSpace' or 'SpaceBar' to Return to Main Menu",
        L""
    };

    if (BootKickerMenu.EntryCount > 0) {
        #if REFIT_DEBUG > 0
        LOG(1, LOG_LINE_NORMAL, L"Displayed previously constructed screen");
        #endif
    }
    else {
        BootKickerMenu.TitleImage = BuiltinIcon (BUILTIN_ICON_TOOL_BOOTKICKER);
        BootKickerMenu.Title = L"BootKicker";
        AddMenuInfoLine (&BootKickerMenu, StrDuplicate (MenuInfo));
        MyFreePool (&MenuInfo);
        AddMenuInfoLine (&BootKickerMenu, L"Needs GOP Capable Fully Compatible GPUs on Apple Firmware");
        AddMenuInfoLine (&BootKickerMenu, L"(Fully Compatible GPUs provide native Apple Boot Screen)");
        AddMenuInfoLine (&BootKickerMenu, L"NB: Hangs and needs physical reboot with other GPUs");
        AddMenuInfoLine (&BootKickerMenu, L"");
        AddMenuInfoLine (&BootKickerMenu, L"BootKicker is from OpenCore and Copyright Acidanthera");
        AddMenuInfoLine (&BootKickerMenu, L"Requires at least one of the files below:");
        AddMenuInfoLine (&BootKickerMenu, L"\\EFI\\tools_x64\\x64_BootKicker.efi");
        AddMenuInfoLine (&BootKickerMenu, L"\\EFI\\tools_x64\\BootKicker_x64.efi");
        AddMenuInfoLine (&BootKickerMenu, L"\\EFI\\tools_x64\\BootKicker.efi");
        AddMenuInfoLine (&BootKickerMenu, L"\\EFI\\tools\\x64_BootKicker.efi");
        AddMenuInfoLine (&BootKickerMenu, L"\\EFI\\tools\\BootKicker_x64.efi");
        AddMenuInfoLine (&BootKickerMenu, L"\\EFI\\tools\\BootKicker.efi");
        AddMenuInfoLine (&BootKickerMenu, L"\\EFI\\x64_BootKicker.efi");
        AddMenuInfoLine (&BootKickerMenu, L"\\EFI\\BootKicker_x64.efi");
        AddMenuInfoLine (&BootKickerMenu, L"\\EFI\\BootKicker.efi");
        AddMenuInfoLine (&BootKickerMenu, L"");
        AddMenuInfoLine (&BootKickerMenu, L"The first file found in the order listed will be used");
        AddMenuInfoLine (&BootKickerMenu, L"You will be returned to the main menu if not found");
        AddMenuInfoLine (&BootKickerMenu, L"");
        AddMenuInfoLine (&BootKickerMenu, L"");
        AddMenuInfoLine (&BootKickerMenu, L"You can get the BootKicker efi file here:");
        AddMenuInfoLine (&BootKickerMenu, L"https://github.com/acidanthera/OpenCorePkg/releases");
        AddMenuInfoLine (&BootKickerMenu, L"https://github.com/dakanji/RefindPlus/tree/GOPFix/BootMaster/tools_x64");
        AddMenuInfoLine (&BootKickerMenu, L"");
        AddMenuInfoLine (&BootKickerMenu, L"");

        AddMenuEntry (&BootKickerMenu, &MenuEntryBootKicker);
        AddMenuEntry (&BootKickerMenu, &MenuEntryReturn);

        MyFreePool (&TempMenuEntry);
    }

    MenuExit = RunGenericMenu (&BootKickerMenu, Style, &DefaultEntry, &ChosenEntry);
    #if REFIT_DEBUG > 0
    LOG(2, LOG_LINE_NORMAL,
        L"Returned '%d' from RunGenericMenu call on '%s' in 'preBootKicker'",
        MenuExit, ChosenEntry->Title
    );
    #endif

    if (ChosenEntry) {
        #if REFIT_DEBUG > 0
        MsgLog ("User Input Received:\n");
        #endif

        if (MyStriCmp (ChosenEntry->Title, L"Load BootKicker") &&
            MenuExit == MENU_EXIT_ENTER
        ) {
            UINTN        i = 0;
            UINTN        k = 0;

            CHAR16       *Names          = BOOTKICKER_FILES;
            CHAR16       *FilePath       = NULL;
            CHAR16       *Description    = ChosenEntry->Title;
            BOOLEAN       FoundTool      = FALSE;
            LOADER_ENTRY *ourLoaderEntry = NULL;

            #if REFIT_DEBUG > 0
            // Log Load BootKicker
            MsgLog ("  - Seek BootKicker\n");
            #endif

            k = 0;
            while ((FilePath = FindCommaDelimited (Names, k++)) != NULL) {
                #if REFIT_DEBUG > 0
                MsgLog ("    * Seek %s:\n", FilePath);
                #endif

                for (i = 0; i < VolumesCount; i++) {
                    if ((Volumes[i]->RootDir != NULL) &&
                        IsValidTool (Volumes[i], FilePath)
                    ) {
                        ourLoaderEntry = AllocateZeroPool (sizeof (LOADER_ENTRY));
                        ourLoaderEntry->me.Title          = Description;
                        ourLoaderEntry->me.Tag            = TAG_SHOW_BOOTKICKER;
                        ourLoaderEntry->me.Row            = 1;
                        ourLoaderEntry->me.ShortcutLetter = 0;
                        ourLoaderEntry->me.Image          = BuiltinIcon (BUILTIN_ICON_TOOL_BOOTKICKER);
                        ourLoaderEntry->LoaderPath        = StrDuplicate (FilePath);
                        ourLoaderEntry->Volume            = Volumes[i];
                        ourLoaderEntry->UseGraphicsMode   = TRUE;

                        FoundTool = TRUE;
                        break;
                    } // if
                } // for

                if (FoundTool) {
                    break;
                }
                else {
                    MyFreePool (&FilePath);
                }
            } // while Names

            if (FoundTool) {
                #if REFIT_DEBUG > 0
                MsgLog ("    ** Success: Found %s\n", FilePath);
                MsgLog ("  - Load BootKicker\n\n");
                #endif

                // Run BootKicker
                StartTool (ourLoaderEntry);
                #if REFIT_DEBUG > 0
                MsgLog ("* WARN: BootKicker Error ... Return to Main Menu\n\n");
                #endif
            }
            else {
                #if REFIT_DEBUG > 0
                MsgLog ("  * WARN: Could Not Find BootKicker ... Return to Main Menu\n\n");
                #endif
            }

            MyFreePool (&FilePath);
        }
        else {
            #if REFIT_DEBUG > 0
            // Log Return to Main Screen
            MsgLog ("  - %s\n\n", ChosenEntry->Title);
            #endif
        } // if
    }
    else {
        #if REFIT_DEBUG > 0
        MsgLog ("WARN: Could Not Get User Input  ... Reload Main Menu\n\n");
        #endif
    } // if
} // VOID preBootKicker()

VOID preCleanNvram (
    VOID
) {
    UINTN              MenuExit;
    INTN               DefaultEntry   = 1;
    MENU_STYLE_FUNC    Style          = GraphicsMenuStyle;
    REFIT_MENU_ENTRY  *ChosenEntry;
    REFIT_MENU_ENTRY  *TempMenuEntry  = CopyMenuEntry (&MenuEntryCleanNvram);
    TempMenuEntry->Image              = BuiltinIcon (BUILTIN_ICON_TOOL_NVRAMCLEAN);
    CHAR16            *MenuInfo       = L"A Tool to Clean/Reset Nvram on Macs";
    REFIT_MENU_SCREEN  CleanNvramMenu = {
        L"Clean NVRAM",
        NULL, 0, &MenuInfo,
        0, &TempMenuEntry,
        0, NULL,
        L"Press 'ESC', 'BackSpace' or 'SpaceBar' to Return to Main Menu",
        L""
    };

    if (CleanNvramMenu.EntryCount > 0) {
        #if REFIT_DEBUG > 0
        LOG(1, LOG_LINE_NORMAL, L"Displayed previously constructed screen");
        #endif
    }
    else {
        CleanNvramMenu.TitleImage = BuiltinIcon (BUILTIN_ICON_TOOL_NVRAMCLEAN);
        CleanNvramMenu.Title = L"Clean NVRAM";
        AddMenuInfoLine (&CleanNvramMenu, StrDuplicate (MenuInfo));
        MyFreePool (&MenuInfo);
        AddMenuInfoLine (&CleanNvramMenu, L"Requires Apple Firmware");
        AddMenuInfoLine (&CleanNvramMenu, L"");
        AddMenuInfoLine (&CleanNvramMenu, L"CleanNvram is from OpenCore and Copyright Acidanthera");
        AddMenuInfoLine (&CleanNvramMenu, L"Requires at least one of the files below:");
        AddMenuInfoLine (&CleanNvramMenu, L"\\EFI\\tools_x64\\x64_CleanNvram.efi");
        AddMenuInfoLine (&CleanNvramMenu, L"\\EFI\\tools_x64\\CleanNvram_x64.efi");
        AddMenuInfoLine (&CleanNvramMenu, L"\\EFI\\tools_x64\\CleanNvram.efi");
        AddMenuInfoLine (&CleanNvramMenu, L"\\EFI\\tools\\x64_CleanNvram.efi");
        AddMenuInfoLine (&CleanNvramMenu, L"\\EFI\\tools\\CleanNvram_x64.efi");
        AddMenuInfoLine (&CleanNvramMenu, L"\\EFI\\tools\\CleanNvram.efi");
        AddMenuInfoLine (&CleanNvramMenu, L"\\EFI\\x64_CleanNvram.efi");
        AddMenuInfoLine (&CleanNvramMenu, L"\\EFI\\CleanNvram_x64.efi");
        AddMenuInfoLine (&CleanNvramMenu, L"\\EFI\\CleanNvram.efi");
        AddMenuInfoLine (&CleanNvramMenu, L"");
        AddMenuInfoLine (&CleanNvramMenu, L"The first file found in the order listed will be used");
        AddMenuInfoLine (&CleanNvramMenu, L"You will be returned to the main menu if not found");
        AddMenuInfoLine (&CleanNvramMenu, L"");
        AddMenuInfoLine (&CleanNvramMenu, L"");
        AddMenuInfoLine (&CleanNvramMenu, L"You can get the CleanNvram efi file here:");
        AddMenuInfoLine (&CleanNvramMenu, L"https://github.com/acidanthera/OpenCorePkg/releases");
        AddMenuInfoLine (&CleanNvramMenu, L"https://github.com/dakanji/RefindPlus/tree/GOPFix/BootMaster/tools_x64");
        AddMenuInfoLine (&CleanNvramMenu, L"");
        AddMenuInfoLine (&CleanNvramMenu, L"");

        AddMenuEntry (&CleanNvramMenu, &MenuEntryCleanNvram);
        AddMenuEntry (&CleanNvramMenu, &MenuEntryReturn);

        MyFreePool (&TempMenuEntry);
    }

    MenuExit = RunGenericMenu (&CleanNvramMenu, Style, &DefaultEntry, &ChosenEntry);
    #if REFIT_DEBUG > 0
    LOG(2, LOG_LINE_NORMAL,
        L"Returned '%d' from RunGenericMenu call on '%s' in 'preCleanNvram'",
        MenuExit, ChosenEntry->Title
    );
    #endif

    if (ChosenEntry) {
        #if REFIT_DEBUG > 0
        MsgLog ("User Input Received:\n");
        #endif

        if (MyStriCmp (ChosenEntry->Title, L"Load CleanNvram") && (MenuExit == MENU_EXIT_ENTER)) {
            UINTN        i = 0;
            UINTN        k = 0;

            CHAR16        *Names           = NVRAMCLEAN_FILES;
            CHAR16        *FilePath        = NULL;
            CHAR16        *Description     = ChosenEntry->Title;
            BOOLEAN        FoundTool       = FALSE;
            LOADER_ENTRY  *ourLoaderEntry  = NULL;

            #if REFIT_DEBUG > 0
            // Log Load CleanNvram
            MsgLog ("  - Seek CleanNvram\n");
            #endif

            k = 0;
            while ((FilePath = FindCommaDelimited (Names, k++)) != NULL) {

                #if REFIT_DEBUG > 0
                MsgLog ("    * Seek %s:\n", FilePath);
                #endif

                for (i = 0; i < VolumesCount; i++) {
                    if ((Volumes[i]->RootDir != NULL) && (IsValidTool (Volumes[i], FilePath))) {
                        ourLoaderEntry = AllocateZeroPool (sizeof (LOADER_ENTRY));
                        ourLoaderEntry->me.Title          = Description;
                        ourLoaderEntry->me.Tag            = TAG_NVRAMCLEAN;
                        ourLoaderEntry->me.Row            = 1;
                        ourLoaderEntry->me.ShortcutLetter = 0;
                        ourLoaderEntry->me.Image          = BuiltinIcon (BUILTIN_ICON_TOOL_NVRAMCLEAN);
                        ourLoaderEntry->LoaderPath        = StrDuplicate (FilePath);
                        ourLoaderEntry->Volume            = Volumes[i];
                        ourLoaderEntry->UseGraphicsMode   = FALSE;

                        FoundTool = TRUE;
                        break;
                    } // if
                } // for

                if (FoundTool) {
                    break;
                }
                else {
                    MyFreePool (&FilePath);
                }
            } // while Names

            if (FoundTool) {
                #if REFIT_DEBUG > 0
                MsgLog ("    ** Success: Found %s\n", FilePath);
                MsgLog ("  - Load CleanNvram\n\n");
                #endif

                ranCleanNvram = TRUE;

                // Run CleanNvram
                StartTool (ourLoaderEntry);

            }
            else {
                #if REFIT_DEBUG > 0
                MsgLog ("  * WARN: Could Not Find CleanNvram ... Return to Main Menu\n\n");
                #endif
            }

            MyFreePool (&FilePath);
        }
        else {
            #if REFIT_DEBUG > 0
            // Log Return to Main Screen
            MsgLog ("  - %s\n\n", ChosenEntry->Title);
            #endif
        } // if
    }
    else {
        #if REFIT_DEBUG > 0
        MsgLog ("WARN: Could Not Get User Input  ... Reload Main Menu\n\n");
        #endif
    } // if
} // VOID preCleanNvram()


VOID AboutRefindPlus (
    VOID
) {
    UINT32   CsrStatus;
    CHAR16  *TempStr         = NULL;
    CHAR16  *FirmwareVendor  = StrDuplicate (VendorInfo);

    #if REFIT_DEBUG > 0
    LOG(1, LOG_LINE_THIN_SEP, L"Displaying About/Info Screen");
    #endif

    if (AboutMenu.EntryCount > 0) {
        #if REFIT_DEBUG > 0
        LOG(1, LOG_LINE_NORMAL, L"Displayed previously constructed screen (Not Updated)");
        #endif
    }
    else {
        AboutMenu.TitleImage = BuiltinIcon (BUILTIN_ICON_FUNC_ABOUT);
        AddMenuInfoLine (&AboutMenu, PoolPrint (L"RefindPlus v%s", REFINDPLUS_VERSION));
        AddMenuInfoLine (&AboutMenu, L"");

        AddMenuInfoLine (&AboutMenu, L"Copyright (c) 2020-2021 Dayo Akanji");
        AddMenuInfoLine (&AboutMenu, L"Portions Copyright (c) 2012-2021 Roderick W. Smith");
        AddMenuInfoLine (&AboutMenu, L"Portions Copyright (c) 2006-2010 Christoph Pfisterer");
        AddMenuInfoLine (&AboutMenu, L"Portions Copyright (c) The Intel Corporation and others");
        AddMenuInfoLine (&AboutMenu, L"Distributed under the terms of the GNU GPLv3 license");
        AddMenuInfoLine (&AboutMenu, L"");

        #if defined (__MAKEWITH_GNUEFI)
        AddMenuInfoLine (&AboutMenu, L"Built with GNU-EFI");
        #else
        AddMenuInfoLine (&AboutMenu, L"Built with TianoCore EDK II");
        #endif

        AddMenuInfoLine (&AboutMenu, L"");

        // More than ~65 causes empty info page on 800x600 display
        LimitStringLength (FirmwareVendor, MAX_LINE_LENGTH);

        AddMenuInfoLine (
            &AboutMenu,
            PoolPrint (
                L"Firmware Vendor: %s %d.%02d",
                FirmwareVendor,
                gST->FirmwareRevision >> 16,
                gST->FirmwareRevision & ((1 << 16) - 1)
            )
        );

        #if defined (EFI32)
        AddMenuInfoLine (&AboutMenu, L"Platform: x86 (32 bit)");
        #elif defined (EFIX64)
        AddMenuInfoLine (&AboutMenu, L"Platform: x86_64 (64 bit)");
        #elif defined (EFIAARCH64)
        AddMenuInfoLine (&AboutMenu, L"Platform: ARM (64 bit)");
        #else
        AddMenuInfoLine (&AboutMenu, L"Platform: Unknown");
        #endif

        if ((gST->Hdr.Revision >> 16) == 1) {
            TempStr = L"EFI";
        }
        else {
            TempStr = L"UEFI";
        }
        AddMenuInfoLine (
            &AboutMenu,
            PoolPrint (
                L"EFI Revision: %s %d.%02d",
                TempStr,
                gST->Hdr.Revision >> 16,
                gST->Hdr.Revision & ((1 << 16) - 1)
            )
        );
        MyFreePool (&TempStr);

        AddMenuInfoLine (
            &AboutMenu,
            PoolPrint (
                L"Secure Boot: %s",
                secure_mode() ? L"active" : L"inactive"
            )
        );

        if (GetCsrStatus (&CsrStatus) == EFI_SUCCESS) {
            RecordgCsrStatus (CsrStatus, FALSE);
            AddMenuInfoLine (&AboutMenu, gCsrStatus);
        }

        TempStr = egScreenDescription();
        AddMenuInfoLine(&AboutMenu, PoolPrint(L"Screen Output: %s", TempStr));
        MyFreePool (&TempStr);

        AddMenuInfoLine (&AboutMenu, L"");
        AddMenuInfoLine (&AboutMenu, L"RefindPlus is a variant of rEFInd");
        AddMenuInfoLine (&AboutMenu, L"https://github.com/dakanji/RefindPlus");
        AddMenuInfoLine (&AboutMenu, L"");
        AddMenuInfoLine (&AboutMenu, L"For information on rEFInd, visit:");
        AddMenuInfoLine (&AboutMenu, L"http://www.rodsbooks.com/refind");
        AddMenuEntry (&AboutMenu, &MenuEntryReturn);
        MyFreePool (&FirmwareVendor);
    }

    RunMenu (&AboutMenu, NULL);
} // VOID AboutRefindPlus()

// Record the loader's name/description in the "PreviousBoot" EFI variable
// if different from what is already stored there.
VOID StoreLoaderName (
    IN CHAR16 *Name
) {
    // Do not set if configured not to
    if (GlobalConfig.IgnorePreviousBoot) {
        return;
    }

    if (Name) {
        EfivarSetRaw (
            &RefindPlusGuid,
            L"PreviousBoot",
            (CHAR8*) Name,
            StrLen (Name) * 2 + 2,
            TRUE
        );
    } // if
} // VOID StoreLoaderName()

// Rescan for boot loaders
VOID RescanAll (
    BOOLEAN DisplayMessage,
    BOOLEAN Reconnect
) {
    #if REFIT_DEBUG > 0
    LOG(1, LOG_LINE_NORMAL, L"Re-scanning Tools and Loaders");
    MsgLog ("INFO: Re-scanning Tools and Loaders\n\n");
    #endif

    FreeList (
        (VOID ***) &(MainMenu.Entries),
        &MainMenu.EntryCount
    );
    MainMenu.Entries     = NULL;
    MainMenu.EntryCount  = 0;

    // ConnectAllDriversToAllControllers() can cause system hangs with some
    // buggy filesystem drivers, so do it only if necessary....
    if (Reconnect) {
        ConnectAllDriversToAllControllers(FALSE);
        ScanVolumes();
    }

    ReadConfig (GlobalConfig.ConfigFilename);
    SetVolumeIcons();
    ScanForBootloaders (DisplayMessage);
    ScanForTools();
} // VOID RescanAll()

#ifdef __MAKEWITH_TIANO

// Minimal initialisation function
static VOID InitializeLib (
    IN EFI_HANDLE         ImageHandle,
    IN EFI_SYSTEM_TABLE  *SystemTable
) {
    gImageHandle  = ImageHandle;
    gST           = SystemTable;
    gBS           = SystemTable->BootServices;
    gRT           = SystemTable->RuntimeServices;

    EfiGetSystemConfigurationTable (
        &gEfiDxeServicesTableGuid,
        (VOID **) &gDS
    );

    // Upgrade EFI_BOOT_SERVICES.HandleProtocol
    OrigHandleProtocol   = gBS->HandleProtocol;
    gBS->HandleProtocol  = HandleProtocolEx;
    gBS->Hdr.CRC32       = 0;
    gBS->CalculateCrc32 (gBS, gBS->Hdr.HeaderSize, &gBS->Hdr.CRC32);
} // VOID InitializeLib()

#endif

// Set up our own Secure Boot extensions....
// Returns TRUE on success, FALSE otherwise
static
BOOLEAN SecureBootSetup (
    VOID
) {
    EFI_STATUS  Status;
    BOOLEAN     Success = FALSE;
    CHAR16     *MsgStr  = NULL;

    #if REFIT_DEBUG > 0
    LOG(1, LOG_LINE_NORMAL, L"Setting up Secure Boot (if applicable)");
    #endif

    if (secure_mode() && ShimLoaded()) {
        #if REFIT_DEBUG > 0
        LOG(2, LOG_LINE_NORMAL, L"Secure boot mode detected with loaded Shim; adding MOK extensions");
        #endif

        Status = security_policy_install();
        if (Status == EFI_SUCCESS) {
            Success = TRUE;
        }
        else {
            MsgStr = StrDuplicate (L"Secure boot disabled ... doing nothing");

            #if REFIT_DEBUG > 0
            LOG(2, LOG_LINE_NORMAL, MsgStr)
            MsgLog ("** WARN: %s\n-----------------\n\n", MsgStr);
            #endif

            refit_call2_wrapper(gST->ConOut->SetAttribute, gST->ConOut, ATTR_ERROR);
            PrintUglyText (MsgStr, NEXTLINE);
            refit_call2_wrapper(gST->ConOut->SetAttribute, gST->ConOut, ATTR_BASIC);

            MyFreePool (&MsgStr);

            PauseForKey();
        }
    }

    return Success;
} // VOID SecureBootSetup()

// Remove our own Secure Boot extensions.
// Returns TRUE on success, FALSE otherwise
static
BOOLEAN SecureBootUninstall (VOID) {
    EFI_STATUS  Status;
    BOOLEAN     Success = TRUE;
    CHAR16     *MsgStr  = NULL;

    if (secure_mode()) {
        Status = security_policy_uninstall();
        if (Status != EFI_SUCCESS) {
            Success = FALSE;
            BeginTextScreen (L"Secure Boot Policy Failure");

            MsgStr = StrDuplicate (
                L"Failed to Uninstall MOK Secure Boot Extensions ... Forcing Shutdown in 9 Seconds"
            );

            #if REFIT_DEBUG > 0
            MsgLog ("%s\n-----------------\n\n", MsgStr);
            #endif

            refit_call2_wrapper(gST->ConOut->SetAttribute, gST->ConOut, ATTR_ERROR);
            PrintUglyText (MsgStr, NEXTLINE);
            refit_call2_wrapper(gST->ConOut->SetAttribute, gST->ConOut, ATTR_BASIC);

            MyFreePool (&MsgStr);

            PauseSeconds(9);

            refit_call4_wrapper(
                gRT->ResetSystem,
                EfiResetShutdown,
                EFI_SUCCESS,
                0,
                NULL
            );
        }
    }

    return Success;
} // BOOLEAN SecureBootUninstall()

// Sets the global configuration filename; will be CONFIG_FILE_NAME unless the
// "-c" command-line option is set, in which case that takes precedence.
// If an error is encountered, leaves the value alone (it should be set to
// CONFIG_FILE_NAME when GlobalConfig is initialized).
static
VOID SetConfigFilename (EFI_HANDLE ImageHandle) {
    EFI_STATUS         Status;
    CHAR16            *Options;
    CHAR16            *FileName;
    CHAR16            *SubString;
    CHAR16            *MsgStr = NULL;
    EFI_LOADED_IMAGE  *Info;

    Status = refit_call3_wrapper(
        gBS->HandleProtocol,
        ImageHandle,
        &LoadedImageProtocol,
        (VOID **) &Info
    );
    if ((Status == EFI_SUCCESS) && (Info->LoadOptionsSize > 0)) {
        #if REFIT_DEBUG > 0
        MsgLog ("Set Config Filename from Command Line Option:\n");
        #endif

        Options = (CHAR16 *) Info->LoadOptions;
        SubString = MyStrStr (Options, L" -c ");
        if (SubString) {
            FileName = StrDuplicate (&SubString[4]);
            if (FileName) {
                LimitStringLength (FileName, 256);
            }

            if (FileExists (SelfDir, FileName)) {
                GlobalConfig.ConfigFilename = FileName;

                #if REFIT_DEBUG > 0
                MsgLog ("  - Config File:- '%s'\n\n", FileName);
                #endif
            }
            else {
                MsgStr = StrDuplicate (L"Specified Config File Not Found");
                #if REFIT_DEBUG > 0
                MsgLog ("** WARN: %s\n", MsgStr);
                #endif
                PrintUglyText (MsgStr, NEXTLINE);

                MyFreePool (&MsgStr);

                MsgStr = StrDuplicate (L"Try Default:- 'config.conf / refind.conf'");
                PrintUglyText (MsgStr, NEXTLINE);

                #if REFIT_DEBUG > 0
                MsgLog ("         %s\n\n", MsgStr);
                #endif

                PrintUglyText (MsgStr, NEXTLINE);

                HaltForKey();
                MyFreePool (&MsgStr);
            } // if/else

            MyFreePool (&FileName);
        } // if
        else {
            MsgStr = StrDuplicate (L"Invalid Load Option");

            #if REFIT_DEBUG > 0
            MsgLog ("** WARN: %s\n", MsgStr);
            #endif
            PrintUglyText (MsgStr, NEXTLINE);

            HaltForKey();
            MyFreePool (&MsgStr);
        }
    } // if
} // VOID SetConfigFilename()

// Adjust the GlobalConfig.DefaultSelection variable: Replace all "+" elements with the
//  PreviousBoot variable, if it's available. If it's not available, delete that element.
static
VOID AdjustDefaultSelection() {
    EFI_STATUS  Status;
    UINTN       i                 = 0;
    CHAR16     *Element           = NULL;
    CHAR16     *NewCommaDelimited = NULL;
    CHAR16     *PreviousBoot      = NULL;

    #if REFIT_DEBUG > 0
    MsgLog ("Adjust Default Selection...\n\n");
    #endif

    while ((Element = FindCommaDelimited (GlobalConfig.DefaultSelection, i++)) != NULL) {
        if (MyStriCmp (Element, L"+")) {
            Status = EfivarGetRaw (
                &RefindPlusGuid,
                L"PreviousBoot",
                (CHAR8 **) &PreviousBoot,
                NULL
            );

            if (Status == EFI_SUCCESS) {
                MyFreePool (&Element);
                Element = PreviousBoot;
                MyFreePool (&PreviousBoot);
            }
            else {
                Element = NULL;
            }
        }

        if (Element && StrLen (Element)) {
            MergeStrings (&NewCommaDelimited, Element, L',');
        }

        MyFreePool (&Element);
    } // while
    MyFreePool (&GlobalConfig.DefaultSelection);
    GlobalConfig.DefaultSelection = NewCommaDelimited;
} // AdjustDefaultSelection()

#if REFIT_DEBUG > 0
// Log basic information (RefindPlus version, EFI version, etc.) to the log file.
static
VOID LogBasicInfo (
    VOID
) {
    EFI_STATUS  Status;
    CHAR16     *TempStr = NULL;
    UINT64      MaximumVariableSize;
    UINT64      MaximumVariableStorageSize;
    UINT64      RemainingVariableStorageSize;
    UINTN       EfiMajorVersion                    = gST->Hdr.Revision >> 16;
    UINTN       HandleCount                        = 0;
    EFI_GUID    ConsoleControlProtocolGuid         = EFI_CONSOLE_CONTROL_PROTOCOL_GUID;
    EFI_GUID    AppleFramebufferInfoProtocolGuid   = APPLE_FRAMEBUFFER_INFO_PROTOCOL_GUID;
    EFI_HANDLE *HandleBuffer                       = NULL;
    APPLE_FRAMEBUFFER_INFO_PROTOCOL  *FramebufferInfo;


    MsgLog ("System Summary...\n");
    MsgLog (
        "EFI Revision:- '%s %d.%02d'",
        (EfiMajorVersion == 1) ? L"EFI" : L"UEFI",
        EfiMajorVersion,
        gST->Hdr.Revision & ((1 << 16) - 1)
    );

    if (((gBS->Hdr.Revision >> 16) == EfiMajorVersion) &&
        ((gRT->Hdr.Revision >> 16) == EfiMajorVersion)
    ) {
        MsgLog ("\n");
    }
    else {
        MsgLog ("\n\n");
        MsgLog ("** WARN: Inconsistent EFI Revisions Detected!!");
        MsgLog ("\n");
        MsgLog ("         RefindPlus Behaviour is not Defined!!");
        MsgLog ("\n\n");
    }

    MsgLog ("Architecture:- ");
    #if defined(EFI32)
        MsgLog ("'x86 (32 bit)'");
    #elif defined(EFIX64)
        MsgLog ("'x86 (64 bit)'");
    #elif defined(EFIAARCH64)
        MsgLog ("'ARM (64 bit)'");
    #else
        MsgLog ("'Unknown'");
    #endif
    MsgLog ("\n");

    switch (GlobalConfig.LegacyType) {
        case LEGACY_TYPE_MAC:
            TempStr = StrDuplicate (L"Mac");
            break;
        case LEGACY_TYPE_UEFI:
            TempStr = StrDuplicate (L"UEFI");
            break;
        case LEGACY_TYPE_NONE:
            TempStr = StrDuplicate (L"Unavailable");
            break;
        default:
            // just in case ... should never happen
            TempStr = StrDuplicate (L"Unknown");
            break;
    }
    MsgLog ("CSM:- '%s'\n", TempStr);
    MyFreePool (&TempStr);

    MsgLog ("Shim:- '%s'\n", ShimLoaded()         ? L"Present" : L"Absent");
    MsgLog ("Secure Boot:- '%s'\n", secure_mode() ? L"Active"  : L"Inactive");

    if (MyStrStr (VendorInfo, L"Apple") != NULL) {
        Status = LibLocateProtocol (&AppleFramebufferInfoProtocolGuid, (VOID *) &FramebufferInfo);
        if (EFI_ERROR (Status)) {
            HandleCount = 0;
        }
        else {
            Status = gBS->LocateHandleBuffer (
                ByProtocol,
                &AppleFramebufferInfoProtocolGuid,
                NULL,
                &HandleCount,
                &HandleBuffer
            );
            if (EFI_ERROR (Status)) {
                HandleCount = 0;
            }

        }
        MsgLog ("Apple Framebuffers:- '%d'\n", HandleCount);
        MyFreePool (&HandleBuffer);
    }

    if ((gRT->Hdr.Revision >> 16) > 1) {
        // NB: QueryVariableInfo() is not supported by EFI 1.x
        MsgLog ("EFI Non-Volatile Storage Info:\n");

        Status = refit_call4_wrapper(
            gRT->QueryVariableInfo,
            EFI_VARIABLE_NON_VOLATILE,
            &MaximumVariableStorageSize,
            &RemainingVariableStorageSize,
            &MaximumVariableSize
        );
        if (EFI_ERROR(Status)) {
            MsgLog ("** WARN: Could not Retrieve Info!!\n");
        }
        else {
            MsgLog ("  - Total Storage         : %ld\n", MaximumVariableStorageSize);
            MsgLog ("  - Remaining Available   : %ld\n", RemainingVariableStorageSize);
            MsgLog ("  - Maximum Variable Size : %ld\n", MaximumVariableSize);
        }
    }

    // Report which video output devices are natively available. We do not actually
    // use them, so just use TempStr as a throwaway pointer to the protocol.
    MsgLog ("Screen Modes:\n");

    Status = LibLocateProtocol (&ConsoleControlProtocolGuid, (VOID **) &TempStr);
    MsgLog ("  - Native Text Mode           : %s", EFI_ERROR (Status) ? L" NO" : L"YES");
    MsgLog ("\n");
    MyFreePool (&TempStr);

    Status = refit_call3_wrapper(
        gBS->HandleProtocol,
        gST->ConsoleOutHandle,
        &gEfiUgaDrawProtocolGuid,
        (VOID **) &TempStr
    );
    MsgLog ("  - Native Graphics Mode (UGA) : %s", EFI_ERROR (Status) ? L" NO" : L"YES");
    MsgLog ("\n");
    MyFreePool (&TempStr);

    Status = refit_call3_wrapper(
        gBS->HandleProtocol,
        gST->ConsoleOutHandle,
        &gEfiGraphicsOutputProtocolGuid,
        (VOID **) &TempStr
    );
    MsgLog ("  - Native Graphics Mode (GOP) : %s", EFI_ERROR (Status) ? L" NO" : L"YES");
    MsgLog ("\n\n");
    MyFreePool (&TempStr);
} // VOID LogBasicInfo()
#endif

//
// main entry point
//
EFI_STATUS EFIAPI efi_main (
    EFI_HANDLE         ImageHandle,
    EFI_SYSTEM_TABLE  *SystemTable
) {
    EFI_STATUS  Status;

    BOOLEAN  MainLoopRunning = TRUE;
    BOOLEAN  MokProtocol     = FALSE;

    REFIT_MENU_ENTRY  *ChosenEntry    = NULL;
    LOADER_ENTRY      *ourLoaderEntry = NULL;
    LEGACY_ENTRY      *ourLegacyEntry = NULL;

    UINTN  i        = 0;
    UINTN  MenuExit = 0;

    EG_PIXEL  BGColor        = COLOR_LIGHTBLUE;
    CHAR16    *MsgStr        = NULL;
    CHAR16    *SelectionName = NULL;

    // Force Native Logging
    ForceNativeLoggging = TRUE;

    // bootstrap
    InitializeLib (ImageHandle, SystemTable);
    Status = InitRefitLib (ImageHandle);

    if (EFI_ERROR (Status)) {
        return Status;
    }

    EFI_TIME Now;
    gRT->GetTime (&Now, NULL);
    NowYear   = Now.Year;
    NowMonth  = Now.Month;
    NowDay    = Now.Day;
    NowHour   = Now.Hour;
    NowMinute = Now.Minute;
    NowSecond = Now.Second;

    if (MyStrStr (gST->FirmwareVendor, L"Apple") != NULL) {
        VendorInfo = StrDuplicate (L"Apple");
    }
    else {
        VendorInfo = PoolPrint (
            L"%s %d.%02d",
            gST->FirmwareVendor,
            gST->FirmwareRevision >> 16,
            gST->FirmwareRevision & ((1 << 16) - 1)
        );
    }

    #if REFIT_DEBUG > 0
    InitBooterLog();

    CONST CHAR16 *ConstDateStr = PoolPrint (
        L"%d-%02d-%02d %02d:%02d:%02d",
        NowYear, NowMonth,
        NowDay, NowHour,
        NowMinute, NowSecond
    );

    MsgLog (
        "Loading RefindPlus v%s on %s Firmware\n",
        REFINDPLUS_VERSION, VendorInfo
    );

#if defined(__MAKEWITH_GNUEFI)
    MsgLog ("Made With:- 'GNU-EFI'\n");
#else
    MsgLog ("Made With:- 'TianoCore EDK II'\n");
#endif
    MsgLog ("Timestamp:- '%s (GMT)'\n\n", ConstDateStr);

    // Log System Details
    LogBasicInfo ();
    #endif

    // read configuration
    CopyMem (GlobalConfig.ScanFor, "ieom       ", NUM_SCAN_OPTIONS);
    FindLegacyBootType();
    if (GlobalConfig.LegacyType == LEGACY_TYPE_MAC) {
        CopyMem (GlobalConfig.ScanFor, "ihebocm    ", NUM_SCAN_OPTIONS);
    }
    SetConfigFilename (ImageHandle);

    // Set Secure Boot Up
    MokProtocol = SecureBootSetup();

    // Scan volumes first to find SelfVolume, which is required by LoadDrivers() and ReadConfig();
    // however, if drivers are loaded, a second call to ScanVolumes() is needed
    // to register the new filesystem (s) accessed by the drivers.
    // Also, ScanVolumes() must be done before ReadConfig(), which needs
    // SelfVolume->VolName.
    ScanVolumes();

    // Read Config first to get tokens that may be required by LoadDrivers();
    if (!FileExists (SelfDir, GlobalConfig.ConfigFilename)) {
        ConfigWarn = TRUE;

        #if REFIT_DEBUG > 0
        MsgLog ("** WARN: Could Not Find RefindPlus Configuration File:- 'config.conf'\n");
        MsgLog ("         Trying rEFInd's Configuration File:- 'refind.conf'\n");
        MsgLog ("         Provide 'config.conf' file to silence this warning\n");
        MsgLog ("         You can rename 'refind.conf' file as 'config.conf'\n");
        MsgLog ("         NB: Will not contain all RefindPlus config tokens\n\n");
        #endif

        GlobalConfig.ConfigFilename = L"refind.conf";
    }
    ReadConfig (GlobalConfig.ConfigFilename);
    AdjustDefaultSelection();

    #if REFIT_DEBUG > 0
    MsgLog ("INFO: LogLevel:- '%d'", GlobalConfig.LogLevel);

    // Show ScanDelay Setting
    MsgLog ("\n");
    MsgLog ("      ScanDelay:- '%d'", GlobalConfig.ScanDelay);

    // Show ReloadGOP Status
    MsgLog ("\n");
    MsgLog ("      ReloadGOP:- ");
    if (GlobalConfig.ReloadGOP) {
        MsgLog ("'YES'");
    }
    else {
        MsgLog ("'NO'");
    }

    // Show SyncAPFS Status
    MsgLog ("\n");
    MsgLog ("      SyncAPFS:- ");
    if (GlobalConfig.SyncAPFS) {
        MsgLog ("'Active'");
    }
    else {
        MsgLog ("'Inactive'");
    }

    // Show NormaliseCSR Status
    MsgLog ("\n");
    MsgLog ("      NormaliseCSR:- ");
    if (GlobalConfig.NormaliseCSR) {
        MsgLog ("'Active'");
    }
    else {
        MsgLog ("'Inactive'");
    }


    // Show TextOnly Status
    MsgLog ("\n");
    MsgLog ("      TextOnly:- ");
    if (GlobalConfig.TextOnly) {
        MsgLog ("'Active'");
    }
    else {
        MsgLog ("'Inactive'");
    }

    // Show ProtectNVRAM Status
    MsgLog ("\n");
    if (MyStrStr (VendorInfo, L"Apple") == NULL) {
        MsgLog ("      ProtectNVRAM:- 'Disabled'");
    }
    else {
        MsgLog ("      ProtectNVRAM:- ");
        if (GlobalConfig.ProtectNVRAM) {
            MsgLog ("'Active'");
        }
        else {
            MsgLog ("'Inactive'");
        }
    }

    // Show ScanOtherESP Status
    MsgLog ("\n");
    MsgLog ("      ScanOtherESP:- ");
    if (GlobalConfig.ScanOtherESP) {
        MsgLog ("'Active'");
    }
    else {
        MsgLog ("'Inactive'");
    }

    // Show IgnorePreviousBoot Status
    MsgLog ("\n");
    MsgLog ("      IgnorePreviousBoot:- ");
    if (GlobalConfig.IgnorePreviousBoot) {
        MsgLog ("'Active'");
    }
    else {
        MsgLog ("'Inactive'");
    }

    #endif

    #ifdef __MAKEWITH_TIANO
    // DA-TAG: Limit to TianoCore
    if (GlobalConfig.SupplyAPFS) {
        Status = RpApfsConnectDevices();

        #if REFIT_DEBUG > 0
        MsgLog ("\n\n");
        MsgLog ("INFO: Supply APFS ... %r", Status);
        #endif
    }
    #endif

    #if REFIT_DEBUG > 0
    // Clear Lines
    if (GlobalConfig.LogLevel > 0) {
        MsgLog ("\n");
    }
    else {
        MsgLog ("\n\n");
    }
    #endif

    // Disable Forced Native Logging
    ForceNativeLoggging = FALSE;

    LoadDrivers();

    #if REFIT_DEBUG > 0
    MsgLog ("Scan Volumes...\n");
    #endif
    ScanVolumes();

    if (GlobalConfig.SpoofOSXVersion && GlobalConfig.SpoofOSXVersion[0] != L'\0') {
        Status = SetAppleOSInfo();

        #if REFIT_DEBUG > 0
        MsgLog ("INFO: Spoof Mac OS Version ... %r\n\n", Status);
        #endif
    }

    // Restore SystemTable if previously amended
    if (SetSysTab) {
        // Reinitialise
        InitializeLib (ImageHandle, SystemTable);

        #if REFIT_DEBUG > 0
        Status = EFI_SUCCESS;
        MsgStr = PoolPrint (L"Restore System Table ... %r", Status);
        LOG(1, LOG_STAR_SEPARATOR, L"%s", MsgStr);
        MsgLog ("INFO: %s", MsgStr);
        MsgLog ("\n\n");
        MyFreePool (&MsgStr);
        #endif
    }

    #if REFIT_DEBUG > 0
    MsgStr = StrDuplicate (L"Initialise Screen");
    LOG(1, LOG_LINE_SEPARATOR, L"%s", MsgStr);
    MsgLog ("%s...", MsgStr);
    MsgLog ("\n");
    MyFreePool (&MsgStr);
    #endif

    InitScreen();

    WarnIfLegacyProblems();
    MainMenu.TimeoutSeconds = GlobalConfig.Timeout;

    // disable EFI watchdog timer
    refit_call4_wrapper(
        gBS->SetWatchdogTimer,
        0x0000, 0x0000, 0x0000,
        NULL
    );

    // further bootstrap (now with config available)
    SetupScreen();
    SetVolumeIcons();
    ScanForBootloaders (FALSE);
    ScanForTools();
    // SetupScreen() clears the screen; but ScanForBootloaders() may display a
    // message that must be deleted, so do so
    BltClearScreen (TRUE);
    pdInitialize();

    if (GlobalConfig.ScanDelay > 0) {
        if (GlobalConfig.ScanDelay > 1) {
            #if REFIT_DEBUG > 0
            LOG(1, LOG_LINE_NORMAL, L"Pausing before re-scan");
            #endif

            egDisplayMessage (
                L"Pausing before disc scan. Please wait....",
                &BGColor, CENTER
            );
        }

        #if REFIT_DEBUG > 0
        MsgLog ("Pause for Scan Delay:\n");
        #endif

        for (i = -1; i < GlobalConfig.ScanDelay; ++i) {
            refit_call1_wrapper(gBS->Stall, 1000000);
        }
        if (i == 1) {
            #if REFIT_DEBUG > 0
            MsgLog ("  - Waited %d Second\n", i);
            #endif
        }
        else {
            #if REFIT_DEBUG > 0
            MsgLog ("  - Waited %d Seconds\n", i);
            #endif
        }
        RescanAll (GlobalConfig.ScanDelay > 1, TRUE);
        BltClearScreen (TRUE);
    } // if

    if (GlobalConfig.DefaultSelection) {
        SelectionName = StrDuplicate (GlobalConfig.DefaultSelection);
    }
    if (GlobalConfig.ShutdownAfterTimeout) {
        MainMenu.TimeoutText = L"Shutdown";
    }

    // show config mismatch warning
    if (ConfigWarn) {
        #if REFIT_DEBUG > 0
        MsgLog ("INFO: Displaying User Warning\n\n");
        #endif

        SwitchToText (FALSE);

        refit_call2_wrapper(gST->ConOut->SetAttribute, gST->ConOut, ATTR_ERROR);
        if (ConfigWarn) {
            PrintUglyText (L"                                                           ", NEXTLINE);
            PrintUglyText (L" WARN: Could Not Find RefindPlus Configuration File        ", NEXTLINE);
            PrintUglyText (L"                                                           ", NEXTLINE);
            PrintUglyText (L"       Trying rEFInd's Configuration File:- 'refind.conf'  ", NEXTLINE);
            PrintUglyText (L"       Provide 'config.conf' file to silence this warning  ", NEXTLINE);
            PrintUglyText (L"       You can rename 'refind.conf' file as 'config.conf'  ", NEXTLINE);
            PrintUglyText (L"       NB: Will not contain all RefindPlus config tokens   ", NEXTLINE);
            PrintUglyText (L"                                                           ", NEXTLINE);
        }
        refit_call2_wrapper(gST->ConOut->SetAttribute, gST->ConOut, ATTR_BASIC);

        PauseSeconds(6);

        #if REFIT_DEBUG > 0
        MsgLog ("INFO: User Warning");

        if (GlobalConfig.ContinueOnWarning) {
            MsgLog (" Acknowledged or Timed Out ...");
        }
        else {
            MsgLog (" Acknowledged ...");
        }
        #endif

        if (egIsGraphicsModeEnabled()) {
            #if REFIT_DEBUG > 0
            MsgLog ("Restore Graphics Mode\n\n");
            #endif

            SwitchToGraphicsAndClear (TRUE);
        }
        else {
            #if REFIT_DEBUG > 0
            MsgLog ("Proceeding\n\n");
            #endif
        }
    }

    // Set CSR if required
    ActiveCSR();

    #if REFIT_DEBUG > 0
    MsgStr = PoolPrint (L"Loaded RefindPlus v%s", REFINDPLUS_VERSION);
    LOG(1, LOG_STAR_SEPARATOR, L"%s", MsgStr);
    MsgLog ("INFO: %s on %s Firmware\n\n", MsgStr, VendorInfo);
    MyFreePool (&MsgStr);
    LOG(1, LOG_LINE_SEPARATOR, L"Entering Main Loop");
    #endif

    while (MainLoopRunning) {
        // Set to false as may not be booting
        IsBoot = FALSE;

        MenuExit = RunMainMenu (&MainMenu, &SelectionName, &ChosenEntry);

        // The Escape key triggers a re-scan operation....
        if (MenuExit == MENU_EXIT_ESCAPE) {
            #if REFIT_DEBUG > 0
            MsgLog ("User Input Received:\n");
            MsgLog ("  - Escape Key Pressed ... Rescan All\n\n");
            #endif

            RescanAll (TRUE, TRUE);
            continue;
        }

        if ((MenuExit == MENU_EXIT_TIMEOUT) && GlobalConfig.ShutdownAfterTimeout) {
            ChosenEntry->Tag = TAG_SHUTDOWN;
        }

        switch (ChosenEntry->Tag) {

            case TAG_NVRAMCLEAN:    // Clean NVRAM
                #if REFIT_DEBUG > 0
                LOG(1, LOG_LINE_NORMAL, L"Cleaning NVRAM");

                MsgLog ("User Input Received:\n");
                if (egIsGraphicsModeEnabled()) {
                    MsgLog ("  - Clean NVRAM\n-----------------\n\n");
                }
                else {
                    MsgLog ("  - Clean NVRAM\n\n");
                }
                #endif

                StartTool ((LOADER_ENTRY *) ChosenEntry);
                break;

            case TAG_PRE_NVRAMCLEAN:    // Clean NVRAM Info
                #if REFIT_DEBUG > 0
                LOG(1, LOG_LINE_THIN_SEP, L"Showing Clean NVRAM Info");

                MsgLog ("User Input Received:\n");
                MsgLog ("  - Show Clean NVRAM Info\n\n");
                #endif

                preCleanNvram();

                // Reboot if CleanNvram was triggered
                if (ranCleanNvram) {
                    #if REFIT_DEBUG > 0
                    LOG(1, LOG_LINE_NORMAL, L"Cleaned NVRAM");

                    MsgLog ("INFO: Cleaned Nvram\n\n");
                    MsgLog ("Terminating Screen:\n");
                    MsgLog ("System Restart...\n");
                    #endif

                    TerminateScreen();

                    #if REFIT_DEBUG > 0
                    LOG(1, LOG_LINE_NORMAL, L"System Restart");

                    MsgLog ("System Restarting\n-----------------\n\n");
                    #endif

                    refit_call4_wrapper(
                        gRT->ResetSystem,
                        EfiResetCold,
                        EFI_SUCCESS,
                        0, NULL
                    );

                    MsgStr = StrDuplicate (L"System Restart FAILED!!");
                    PrintUglyText (MsgStr, NEXTLINE);

                    #if REFIT_DEBUG > 0
                    LOG(1, LOG_THREE_STAR_SEP, MsgStr);
                    MsgLog ("INFO: %s\n\n", MsgStr);
                    #endif

                    MyFreePool (&MsgStr);

                    PauseForKey();

                    MainLoopRunning = FALSE;   // just in case we get this far
                }
                break;

            case TAG_SHOW_BOOTKICKER:    // Apple Boot Screen
                #if REFIT_DEBUG > 0
                LOG(1, LOG_LINE_NORMAL, L"Loading Apple Boot Screen");

                MsgLog ("User Input Received:\n");
                if (egIsGraphicsModeEnabled()) {
                    MsgLog ("  - Load Apple Boot Screen\n-----------------\n\n");
                }
                else {
                    MsgLog ("  - Load Apple Boot Screen\n\n");
                }
                #endif

                ourLoaderEntry = (LOADER_ENTRY *) ChosenEntry;
                ourLoaderEntry->UseGraphicsMode = TRUE;

                StartTool (ourLoaderEntry);
                break;

            case TAG_PRE_BOOTKICKER:    // Apple Boot Screen Info
                #if REFIT_DEBUG > 0
                LOG(1, LOG_LINE_THIN_SEP, L"Showing BootKicker Info");

                MsgLog ("User Input Received:\n");
                MsgLog ("  - Show BootKicker Info\n\n");
                #endif

                preBootKicker();
                break;

            case TAG_REBOOT:    // Reboot
                #if REFIT_DEBUG > 0
                MsgLog ("User Input Received:\n");
                if (egIsGraphicsModeEnabled()) {
                    MsgLog ("  - System Restart\n-----------------\n\n");
                }
                else {
                    MsgLog ("  - System Restart\n\n");
                }
                #endif

                TerminateScreen();

                #if REFIT_DEBUG > 0
                LOG(1, LOG_LINE_SEPARATOR, L"Restarting System");
                #endif

                refit_call4_wrapper(
                    gRT->ResetSystem,
                    EfiResetCold,
                    EFI_SUCCESS,
                    0, NULL
                );

                #if REFIT_DEBUG > 0
                LOG(1, LOG_THREE_STAR_SEP, L"Restart FAILED!!");
                #endif

                MainLoopRunning = FALSE;   // just in case we get this far
                break;

            case TAG_SHUTDOWN: // Shut Down
                #if REFIT_DEBUG > 0
                LOG(1, LOG_LINE_SEPARATOR, L"Shutting System Down");

                MsgLog ("User Input Received:\n");
                if (egIsGraphicsModeEnabled()) {
                    MsgLog ("  - System Shutdown\n-----------------\n\n");
                }
                else {
                    MsgLog ("  - System Shutdown\n\n");
                }
                #endif

                TerminateScreen();

                refit_call4_wrapper(
                    gRT->ResetSystem,
                    EfiResetShutdown,
                    EFI_SUCCESS,
                    0, NULL
                );

                #if REFIT_DEBUG > 0
                LOG(1, LOG_THREE_STAR_SEP, L"Shutdown FAILED!!");
                #endif

                MainLoopRunning = FALSE;   // just in case we get this far
                break;

            case TAG_ABOUT:    // About RefindPlus
                #if REFIT_DEBUG > 0
                MsgLog ("User Input Received:\n");
                MsgLog ("  - Show 'About RefindPlus' Page\n\n");
                #endif

                AboutRefindPlus();

                #if REFIT_DEBUG > 0
                MsgLog ("User Input Received:\n");
                MsgLog ("  - Exit 'About RefindPlus' Page\n\n");
                #endif

                break;

            case TAG_LOADER:   // Boot OS via .EFI Loader
                ourLoaderEntry = (LOADER_ENTRY *) ChosenEntry;

                // Fix undetected Mac OS
                if (MyStrStr (ourLoaderEntry->Title, L"Mac OS") == NULL &&
                    MyStrStr (ourLoaderEntry->LoaderPath, L"System\\Library\\CoreServices") != NULL
                ) {
                    if (MyStriCmp (ourLoaderEntry->Volume->VolName, L"PreBoot")) {
                        ourLoaderEntry->Title = L"Mac OS";
                    }
                    else {
                        ourLoaderEntry->Title = L"RefindPlus";
                    }
                }

                // Fix undetected Windows
                if (MyStrStr (ourLoaderEntry->Title, L"Windows") == NULL &&
                    MyStrStr (ourLoaderEntry->LoaderPath, L"EFI\\Microsoft\\Boot") != NULL
                ) {
                    ourLoaderEntry->Title = L"Windows (UEFI)";
                }

                // Use multiple instaces of "User Input Received:"

                if (MyStrStr (ourLoaderEntry->Title, L"OpenCore") != NULL) {
                    if (!ourLoaderEntry->UseGraphicsMode) {
                        ourLoaderEntry->UseGraphicsMode = GlobalConfig.GraphicsFor & GRAPHICS_FOR_OPENCORE;
                    }

                    #if REFIT_DEBUG > 0
                    LOG(1, LOG_LINE_THIN_SEP,
                        L"Loading OpenCore Instance:- '%s%s'",
                        ourLoaderEntry->Volume->VolName,
                        ourLoaderEntry->LoaderPath
                    );

                    MsgLog ("User Input Received:\n");
                    MsgLog (
                        "  - Load OpenCore Instance:- '%s%s'",
                        ourLoaderEntry->Volume->VolName,
                        ourLoaderEntry->LoaderPath
                    );
                    #endif

                    // Filter out the 'APPLE_INTERNAL' CSR bit if required
                    FilterCSR();
                }
                else if (MyStrStr (ourLoaderEntry->Title, L"Clover") != NULL) {
                    if (!ourLoaderEntry->UseGraphicsMode) {
                        ourLoaderEntry->UseGraphicsMode = GlobalConfig.GraphicsFor & GRAPHICS_FOR_CLOVER;
                    }

                    #if REFIT_DEBUG > 0
                    LOG(1, LOG_LINE_THIN_SEP,
                        L"Loading Clover Instance:- '%s%s'",
                        ourLoaderEntry->Volume->VolName,
                        ourLoaderEntry->LoaderPath
                    );

                    MsgLog ("User Input Received:\n");
                    MsgLog (
                        "  - Load Clover Instance:- '%s%s'",
                        ourLoaderEntry->Volume->VolName,
                        ourLoaderEntry->LoaderPath
                    );
                    #endif

                    // Filter out the 'APPLE_INTERNAL' CSR bit if required
                    FilterCSR();
                }
                else if (MyStrStr (ourLoaderEntry->Title, L"Mac OS") != NULL) {
                    #if REFIT_DEBUG > 0
                    MsgLog ("User Input Received:\n");
                    if (ourLoaderEntry->Volume->VolName) {
                        LOG(1, LOG_LINE_THIN_SEP,
                            L"Booting Mac OS from '%s'",
                            ourLoaderEntry->Volume->VolName
                        );

                        MsgLog ("  - Boot Mac OS from '%s'", ourLoaderEntry->Volume->VolName);
                    }
                    else {
                        MsgLog ("  - Boot Mac OS:- '%s'", ourLoaderEntry->LoaderPath);

                        LOG(1, LOG_LINE_THIN_SEP,
                            L"Booting Mac OS:- '%s'",
                            ourLoaderEntry->LoaderPath
                        );
                    }
                    #endif

                    // Enable TRIM on non-Apple SSDs if configured to
                    if (GlobalConfig.ForceTRIM) {
                        ForceTRIM();
                    }

                    // Set Mac boot args if configured to
                    // Also disables AMFI if this is configured
                    // Also disables Mac OS compatibility check if this is configured
                    if (GlobalConfig.SetBootArgs && GlobalConfig.SetBootArgs[0] != L'\0') {
                        SetBootArgs();
                    }
                    else {
                        if (GlobalConfig.DisableAMFI) {
                            // Disable AMFI if configured to
                            // Also disables Mac OS compatibility check if this is configured
                            DisableAMFI();
                        }
                        else if (GlobalConfig.DisableCompatCheck) {
                            // Disable Mac OS compatibility check if configured to
                            DisableCompatCheck();
                        }
                    }

                    // Filter out the 'APPLE_INTERNAL' CSR bit if required
                    FilterCSR();

                    // Re-Map OpenProtocol
                    ReMapOpenProtocol();
                }
                else if (MyStrStr (ourLoaderEntry->Title, L"Windows") != NULL) {
                    if (GlobalConfig.ProtectNVRAM &&
                        MyStrStr (VendorInfo, L"Apple") != NULL
                    ) {
                        // Protect Mac NVRAM from UEFI Windows
                        MapSetVariable (SystemTable);
                    }

                    #if REFIT_DEBUG > 0
                    CHAR16 *WinType;
                    MsgLog ("User Input Received:\n");
                    if (MyStrStr (ourLoaderEntry->Title, L"UEFI") != NULL) {
                        WinType = L"UEFI";
                    }
                    else {
                        WinType = L"Legacy";
                    }
                    if (ourLoaderEntry->Volume->VolName) {
                        MsgStr = PoolPrint (
                            L"Boot %s Windows from '%s'",
                            WinType,
                            ourLoaderEntry->Volume->VolName
                        );
                        LOG(1, LOG_LINE_THIN_SEP, L"%s", MsgStr);
                        MsgLog ("  - %s");
                        MyFreePool (&MsgStr);
                    }
                    else {
                        MsgStr = PoolPrint (
                            L"Boot %s Windows:- '%s'",
                            WinType,
                            ourLoaderEntry->LoaderPath
                        );
                        LOG(1, LOG_LINE_THIN_SEP, L"%s", MsgStr);
                        MsgLog ("  - %s");
                        MyFreePool (&MsgStr);
                    }
                    #endif
                }
                else {
                    #if REFIT_DEBUG > 0
                    MsgStr = PoolPrint (
                        L"Boot OS via EFI Loader:- '%s%s'",
                        ourLoaderEntry->Volume->VolName,
                        ourLoaderEntry->LoaderPath
                    );
                    LOG(1, LOG_LINE_THIN_SEP, L"%s", MsgStr);
                    MsgLog ("User Input Received:\n");
                    MsgLog ("  - %s");
                    MyFreePool (&MsgStr);
                    #endif
                }

                #if REFIT_DEBUG > 0
                MsgLog ("\n-----------------\n\n");
                #endif

                StartLoader (ourLoaderEntry, SelectionName);
                break;

            case TAG_LEGACY:   // Boot legacy OS
                ourLegacyEntry = (LEGACY_ENTRY *) ChosenEntry;

                #if REFIT_DEBUG > 0
                MsgLog ("User Input Received:\n");
                #endif

                if (MyStrStr (ourLegacyEntry->Volume->OSName, L"Windows") != NULL) {
                    if (GlobalConfig.ProtectNVRAM &&
                        MyStrStr (VendorInfo, L"Apple") != NULL
                    ) {
                        // Protect Mac NVRAM from UEFI Windows
                        MapSetVariable (SystemTable);
                    }

                    #if REFIT_DEBUG > 0
                    MsgStr = PoolPrint (
                        L"Boot %s from '%s'",
                        ourLegacyEntry->Volume->OSName,
                        ourLegacyEntry->Volume->VolName
                    );
                    LOG(1, LOG_LINE_THIN_SEP, L"%s", MsgStr);
                    MsgLog ("  - %s");
                    MyFreePool (&MsgStr);
                    #endif
                }
                else {
                    #if REFIT_DEBUG > 0
                    MsgStr = PoolPrint (
                        L"Boot Legacy OS:- '%s'",
                        ourLegacyEntry->Volume->OSName
                    );
                    LOG(1, LOG_LINE_THIN_SEP, L"%s", MsgStr);
                    MsgLog ("  - %s");
                    MyFreePool (&MsgStr);
                    #endif
                }

                #if REFIT_DEBUG > 0
                if (egIsGraphicsModeEnabled()) {
                    MsgLog ("\n-----------------\n\n");
                }
                else {
                    MsgLog ("\n\n");
                }
                #endif

                StartLegacy (ourLegacyEntry, SelectionName);
                break;

            case TAG_LEGACY_UEFI: // Boot a legacy OS on a non-Mac
                ourLegacyEntry = (LEGACY_ENTRY *) ChosenEntry;

                #if REFIT_DEBUG > 0
                MsgStr = PoolPrint (
                    L"Boot Legacy UEFI:- '%s'",
                    ourLegacyEntry->Volume->OSName
                );
                LOG(1, LOG_LINE_THIN_SEP, L"%s", MsgStr);
                MsgLog ("User Input Received:\n");
                MsgLog ("  - %s");
                MyFreePool (&MsgStr);

                if (egIsGraphicsModeEnabled()) {
                    MsgLog ("\n-----------------\n\n");
                }
                else {
                    MsgLog ("\n\n");
                }
                #endif

                StartLegacyUEFI (ourLegacyEntry, SelectionName);
                break;

            case TAG_TOOL:     // Start a EFI tool
                ourLoaderEntry = (LOADER_ENTRY *) ChosenEntry;

                #if REFIT_DEBUG > 0
                MsgStr = PoolPrint (
                    L"Start EFI Tool:- '%s'",
                    ourLoaderEntry->LoaderPath
                );
                LOG(1, LOG_LINE_THIN_SEP, L"%s", MsgStr);
                MsgLog ("User Input Received:\n");
                MsgLog ("  - %s");
                MsgLog ("\n\n");
                MyFreePool (&MsgStr);
                #endif

                if (MyStrStr (ourLoaderEntry->Title, L"Boot Screen") != NULL) {
                    ourLoaderEntry->UseGraphicsMode = TRUE;
                }

                StartTool (ourLoaderEntry);
                break;

            case TAG_FIRMWARE_LOADER:  // Reboot to a loader defined in the EFI UseNVRAM

                #if REFIT_DEBUG > 0
                MsgLog ("User Input Received:\n");
                MsgLog ("  - Reboot into Loader\n-----------------\n\n");
                #endif

                RebootIntoLoader ((LOADER_ENTRY *) ChosenEntry);
                break;

            case TAG_HIDDEN:  // Manage hidden tag entries

                #if REFIT_DEBUG > 0
                MsgLog ("User Input Received:\n");
                MsgLog ("  - Manage Hidden Tag Entries\n\n");
                #endif

                ManageHiddenTags();

                #if REFIT_DEBUG > 0
                MsgLog ("User Input Received:\n");
                MsgLog ("  - Exit Hidden Tags Page\n\n");
                #endif

                break;

            case TAG_EXIT:    // Terminate RefindPlus

                #if REFIT_DEBUG > 0
                MsgLog ("User Input Received:\n");
                if (egIsGraphicsModeEnabled()) {
                    MsgLog ("  - Terminate RefindPlus\n-----------------\n\n");
                }
                else {
                    MsgLog ("  - Terminate RefindPlus\n\n");
                }
                #endif

                if ((MokProtocol) && !SecureBootUninstall()) {
                   MainLoopRunning = FALSE;   // just in case we get this far
                }
                else {
                   BeginTextScreen (L" ");
                   return EFI_SUCCESS;
                }
                break;

            case TAG_FIRMWARE: // Reboot into firmware's user interface

                #if REFIT_DEBUG > 0
                MsgLog ("User Input Received:\n");
                if (egIsGraphicsModeEnabled()) {
                    MsgLog ("  - Reboot into Firmware\n-----------------\n\n");
                }
                else {
                    MsgLog ("  - Reboot into Firmware\n\n");
                }
                #endif

                RebootIntoFirmware();
                break;

            case TAG_CSR_ROTATE:

                #if REFIT_DEBUG > 0
                MsgLog ("User Input Received:\n");
                MsgLog ("  - Toggle Mac CSR\n");
                #endif

                RotateCsrValue();
                break;

            case TAG_INSTALL:

                #if REFIT_DEBUG > 0
                MsgLog ("User Input Received:\n");
                if (egIsGraphicsModeEnabled()) {
                    MsgLog ("  - Install RefindPlus\n-----------------\n\n");
                }
                else {
                    MsgLog ("  - Install RefindPlus\n\n");
                }
                #endif

                InstallRefindPlus();
                break;

            case TAG_BOOTORDER:

                #if REFIT_DEBUG > 0
                MsgLog ("User Input Received:\n");
                MsgLog ("  - Manage Boot Order\n\n");
                #endif

                ManageBootorder();

                #if REFIT_DEBUG > 0
                MsgLog ("User Input Received:\n");
                MsgLog ("  - Exit Manage Boot Order Page\n\n");
                #endif

                break;
        } // switch()
    } // while()
    MyFreePool (&SelectionName);

    // If we end up here, things have gone wrong. Try to reboot, and if that
    // fails, go into an endless loop.
    #if REFIT_DEBUG > 0
    LOG(1, LOG_LINE_SEPARATOR, L"Main loop has exited, but it should not have!!");

    MsgLog ("Fallback: System Restart...\n");
    MsgLog ("Screen Termination:\n");
    #endif

    TerminateScreen();

    #if REFIT_DEBUG > 0
    MsgLog ("System Reset:\n\n");
    #endif

    refit_call4_wrapper(
        gRT->ResetSystem,
        EfiResetCold,
        EFI_SUCCESS,
        0, NULL
    );

    #if REFIT_DEBUG > 0
    LOG(1, LOG_THREE_STAR_SEP, L"Shutdown after main loop exit has FAILED!!");
    #endif

    SwitchToText (FALSE);

    MsgStr = StrDuplicate (L"INFO: Reboot Failed ... Entering Endless Idle Loop");

    refit_call2_wrapper(
        gST->ConOut->SetAttribute,
        gST->ConOut,
        ATTR_ERROR
    );
    PrintUglyText (MsgStr, NEXTLINE);
    refit_call2_wrapper(
        gST->ConOut->SetAttribute,
        gST->ConOut,
        ATTR_BASIC
    );

    #if REFIT_DEBUG > 0
    MsgLog ("%s\n-----------------\n\n", MsgStr);
    #endif

    MyFreePool (&MsgStr);

    PauseForKey();
    EndlessIdleLoop();

    return EFI_SUCCESS;
} // EFI_STATUS EFIAPI efi_main()
