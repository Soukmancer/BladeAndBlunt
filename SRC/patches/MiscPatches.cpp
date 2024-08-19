#include "patches/MiscPatches.h"
#include "Hooks.h"
#include "Cache.h"

bool MiscPatches::InstallScalePatch() 
{
	auto& trampoline = SKSE::GetTrampoline();
	_GetScaleFunction = trampoline.write_call<5>(Hooks::Scale_Patch_Hook.address(), GetScale);

	logger::info("Installed hook for scale patch");
	return true;
}

float MiscPatches::GetScale(RE::TESObjectREFR* a1)
{
	auto scale = _GetScaleFunction(a1);
	if (skyrim_cast<RE::Actor*>(a1) == RE::PlayerCharacter::GetSingleton()) {
		return 1.0f;
	} else {
		return scale;
	}
}
