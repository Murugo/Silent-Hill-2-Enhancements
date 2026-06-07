/**
* Copyright (C) 2026 Murugo
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

namespace {
    constexpr int kEnteredFloodedHotelBasementGameFlag = 0x1F6;
}

// Fixes James' movement speed in select flooded rooms
void RunHotelWaterMoveSpeed()
{
    static BYTE* EnableWaterMoveSpeed = nullptr;
    if (!EnableWaterMoveSpeed)
    {
        RUNONCE();

        constexpr BYTE SearchBytes[]{ 0x83, 0xC8, 0xFF, 0x83, 0xF8, 0xFF, 0x0F, 0x95, 0xC1, 0xC6, 0x05 };
        EnableWaterMoveSpeed = (BYTE*)ReadSearchedAddresses(0x005469ED, 0x00546D1D, 0x0054663D, SearchBytes, sizeof(SearchBytes), 0x0B, __FUNCTION__);
        if (!EnableWaterMoveSpeed)
        {
            Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
            return;
        }
    }

    const DWORD RoomID = GetRoomID();
    if (RoomID == R_HTL_ALT_ELEVATOR && CheckGameFlag(kEnteredFloodedHotelBasementGameFlag))
    {
        *EnableWaterMoveSpeed = 1;
    }
    else if (RoomID == R_HTL_ALT_EMPLOYEE_STAIRS)
    {
        *EnableWaterMoveSpeed = GetJamesPosY() > -100.0f ? 1 : 0;
    }
    else if (RoomID == R_STRANGE_AREA_2_B)
    {
        *EnableWaterMoveSpeed = GetJamesPosY() > -50.0f ? 1 : 0;
    }
}
