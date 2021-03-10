/** @file
Copyright (C) 2020, vit9696. All rights reserved.

Modified 2021, Dayo Akanji. (sf.net/u/dakanji/profile)

  All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include "RpApfsInternal.h"
#include "RpApfsLib.h"
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Protocol/BlockIo.h>
#include <Protocol/PartitionInfo.h>

#include "../../include/refit_call_wrapper.h"

EFI_STATUS
RpApfsConnectParentDevice (
    VOID
) {
    EFI_STATUS       Status;
    EFI_STATUS       Status2;
    UINTN            HandleCount;
    EFI_HANDLE       *HandleBuffer;
    EFI_DEVICE_PATH  *ParentDevicePath;
    EFI_DEVICE_PATH  *ChildDevicePath;
    UINTN            Index;
    UINTN            PrefixLength;

    HandleCount = 0;
    Status = gBS->LocateHandleBuffer (
        ByProtocol,
        &gEfiBlockIoProtocolGuid,
        NULL,
        &HandleCount,
        &HandleBuffer
    );

    ParentDevicePath = NULL;
    PrefixLength     = 0;
    if (Handle != NULL) {
        Status2 = gBS->HandleProtocol (
            Handle,
            &gEfiDevicePathProtocolGuid,
            (VOID **) &ParentDevicePath
        );
        if (!EFI_ERROR (Status2)) {
            PrefixLength = GetDevicePathSize (ParentDevicePath) - END_DEVICE_PATH_LENGTH;
        }
        else {
            ParentDevicePath = NULL;
        }
    }

    if (!EFI_ERROR (Status)) {
        Status = EFI_NOT_FOUND;

        for (Index = 0; Index < HandleCount; ++Index) {
            if (ParentDevicePath != NULL) {
                if (PrefixLength > 0) {
                    Status2 = gBS->HandleProtocol (
                        HandleBuffer[Index],
                        &gEfiDevicePathProtocolGuid,
                        (VOID **) &ChildDevicePath
                    );

                    if (EFI_ERROR (Status2)) {
                        continue;
                    }

                    if (CompareMem (ParentDevicePath, ChildDevicePath, PrefixLength) == 0) {

                    }
                    else {
                        continue;
                    }
                }
            }

            Status2 = OcApfsConnectHandle (
                HandleBuffer[Index],
                VerifyPolicy
            );

            if (!EFI_ERROR (Status2)) {
                Status = Status2;
            }
        }

        FreePool (HandleBuffer);
    }

    return Status;
}

EFI_STATUS
RpApfsConnectDevices (
    VOID
) {
    EFI_STATUS  Status;
    VOID        *PartitionInfoInterface;


    Status = refit_call3_wrapper(
        gBS->LocateProtocol,
        &gEfiPartitionInfoProtocolGuid,
        NULL,
        &PartitionInfoInterface
    );
    Status = RpApfsConnectParentDevice();

    return Status;
}
