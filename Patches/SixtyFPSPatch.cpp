/**
* Copyright (C) 2022 mercury501, Murugo
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

#include "External\injector\include\injector\injector.hpp"
#include "External\injector\include\injector\calling.hpp"
#include "External\injector\include\injector\hooking.hpp"
#include "External\injector\include\injector\utility.hpp"
#include "External\Hooking.Patterns\Hooking.Patterns.h"
#include "Logging\Logging.h"
#include "Patches.h"
#include "Common/Utils.h"
#include "SixtyFPSPatch.h"

const float MotionBlurOvrd = 0.125f;
const float WaterOvrd = 0.0166665f;
const float LowHealthOvrd = 0.0166665f;
const float StaircaseLightingOvrd = 0.00027777222565f;
const float WaterLowOvrd = 20.f;
const float WaterRisOvrd = 5.333333492f;
const float BugRoomFlashlightOvrd = 0.093333332985f;
const int16_t FlashlightOnOvrd= 0x78;


injector::hook_back<float(__cdecl*)(void)> GetFogAnimationRate;
injector::hook_back<float(__cdecl*)(void)> GetBulletShellAnimationRateOne;
injector::hook_back<float(__cdecl*)(void)> GetBulletShellAnimationRateTwo;
injector::hook_back<float(__cdecl*)(void)> GetBulletShellAnimationRateThree;
injector::hook_back<float(__cdecl*)(void)> GetBulletShellAnimationRateFour;

bool once = true;

float MotionBlurValue = 0.25f;

void PatchWater()
{
	Logging::Log() << "Patching water animation speed...";

	float* WaterAnimationSpeedPtr = GetWaterAnimationSpeedPointer();

	UpdateMemoryAddress(WaterAnimationSpeedPtr, &WaterOvrd, sizeof(float));
}

void PatchFlashlightOnSpeed()
{
	Logging::Log() << "Patching flashlight speed...";

	int16_t* FlashlightOnSpeedPtr = GetFlashlightOnSpeedPointer();

	UpdateMemoryAddress(FlashlightOnSpeedPtr, &FlashlightOnOvrd, sizeof(int16_t));
}

void PatchLowHealthIndicatorFlash()
{
	Logging::Log() << "Patching low health indicator flashing speed...";

	float* LowHealthIndicatorSpeedPtr = GetLowHealthIndicatorFlashSpeedPointer();
	
	UpdateMemoryAddress(LowHealthIndicatorSpeedPtr, &LowHealthOvrd, sizeof(float));
}

void PatchStaircaseFlamesLighting()
{
	Logging::Log() << "Patching flame staircase lighting...";

	float* StaircaseFlamesLightingPtr = GetStaircaseFlamesLightingPointer();

	UpdateMemoryAddress(StaircaseFlamesLightingPtr, &StaircaseLightingOvrd, sizeof(float));
}

void PatchWaterLevelSpeed()
{
	Logging::Log() << "Patching water level speed...";

	float* LoweringStepsPtr = GetWaterLevelLoweringStepsPointer();
	float* RisingStepsPtr = GetWaterLevelRisingStepsPointer();

	UpdateMemoryAddress(LoweringStepsPtr, &WaterLowOvrd, sizeof(float));
	UpdateMemoryAddress(RisingStepsPtr, &WaterRisOvrd, sizeof(float));
}

void PatchBugRoomFlashlight()
{
	Logging::Log() << "Patching bug room flashlight...";

	float* BugRoomFlashlightPtr = GetBugRoomFlashlightFixPointer();

	UpdateMemoryAddress(BugRoomFlashlightPtr, &BugRoomFlashlightOvrd, sizeof(float));
}

void PatchFMV()
{
	Logging::Log() << "Patching fmv speed...";

	uint8_t* FMVFixAddr = GetSixtyFPSFMVFixPointer();
	
	UpdateMemoryAddress(FMVFixAddr, "\x90\x90\x90\x90\x90", 5);
}

float __cdecl GetHalvedAnimationRate_Hook()
{
	return GetFogAnimationRate.fun() / 2;
}

float __cdecl GetDoubledAnimationRate_Hook()
{
	return GetFogAnimationRate.fun() * 2;
}

void PatchSixtyFPS()
{
	Logging::Log() << "Applying Fixes for 60 FPS...";

	Logging::Log() << "Hooking Fog Animation...";
	constexpr BYTE FogAnimationRateSearchBytes[]{ 0x83, 0xC4, 0x04, 0x84, 0xDB, 0x5B, 0x8A, 0x54, 0x24, 0x0C };
	DWORD* AnimationRate = (DWORD*)SearchAndGetAddresses(0x4890D0, 0x489370, 0x489580, FogAnimationRateSearchBytes, sizeof(FogAnimationRateSearchBytes), 0x50);
	if (AnimationRate)
	{
		GetFogAnimationRate.fun = injector::MakeCALL(AnimationRate, GetHalvedAnimationRate_Hook, true).get();
	}
	else 
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Animation Rate Function address!";
	}
	
	Logging::Log() << "Hooking Bullet Animation...";
	constexpr BYTE BulletAnimationOneSearchBytes[]{ 0x88, 0x8C, 0x24, 0xB6, 0x00, 0x00, 0x00, 0x89 };
	DWORD* BulletAnimationOne = (DWORD*)SearchAndGetAddresses(0x4F0E12, 0x4F10C2, 0x4F0982, BulletAnimationOneSearchBytes, sizeof(BulletAnimationOneSearchBytes), 0xE3);
	if (BulletAnimationOne)
	{
		GetBulletShellAnimationRateOne.fun = injector::MakeCALL(BulletAnimationOne, GetDoubledAnimationRate_Hook, true).get();
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Bullet Animation Rate One Function address!";
	}

	constexpr BYTE BulletAnimationTwoSearchBytes[]{ 0xD8, 0x45, 0x08, 0xD9, 0x5D, 0x08, 0xD9, 0x44, 0x24, 0x1C };
	DWORD* BulletAnimationTwo = (DWORD*)SearchAndGetAddresses(0x4F0EE5, 0x4F1195, 0x4F0A55, BulletAnimationTwoSearchBytes, sizeof(BulletAnimationTwoSearchBytes), -0x39);
	if (BulletAnimationTwo)
	{
		GetBulletShellAnimationRateTwo.fun = injector::MakeCALL(BulletAnimationTwo, GetDoubledAnimationRate_Hook, true).get();
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Bullet Animation Rate Two Function address!";
	}
	
	constexpr BYTE BulletAnimationThreeSearchBytes[]{ 0xD8, 0x45, 0x08, 0xD9, 0x5D, 0x08, 0xD9, 0x44, 0x24, 0x1C };
	DWORD* BulletAnimationThree = (DWORD*)SearchAndGetAddresses(0x4F0EE5, 0x4F1195, 0x4F0A55, BulletAnimationThreeSearchBytes, sizeof(BulletAnimationThreeSearchBytes), 0x2D);
	if (BulletAnimationThree)
	{
		GetBulletShellAnimationRateThree.fun = injector::MakeCALL(BulletAnimationThree, GetDoubledAnimationRate_Hook, true).get();
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Bullet Animation Rate Three Function address!";
	}

	constexpr BYTE BulletAnimationFourSearchBytes[]{ 0xD8, 0x45, 0x08, 0xD9, 0x5D, 0x08, 0xD9, 0x44, 0x24, 0x1C };
	DWORD* BulletAnimationFour = (DWORD*)SearchAndGetAddresses(0x4F0EE5, 0x4F1195, 0x4F0A55, BulletAnimationFourSearchBytes, sizeof(BulletAnimationFourSearchBytes), 0x3E);
	if (BulletAnimationFour)
	{
		GetBulletShellAnimationRateFour.fun = injector::MakeCALL(BulletAnimationFour, GetDoubledAnimationRate_Hook, true).get();
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Bullet Animation Rate Four Function address!";
	}

	Logging::Log() << "Patching motion blur...";
	MotionBlurValue = MotionBlurOvrd;

	PatchWater();

	PatchFlashlightOnSpeed();

	PatchLowHealthIndicatorFlash();

	PatchStaircaseFlamesLighting();

	PatchWaterLevelSpeed();

	PatchPrisonerTimer();

	PatchSprayEffect();

	PatchMapTranscription();

	PatchBugRoomFlashlight();

    PatchHoldDamage();

	PatchFMV();

	Logging::Log() << "Done applying 60 FPS fixes...";

}