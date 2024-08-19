#pragma once

namespace Hooks
{
	inline static REL::Relocation<std::uintptr_t> Scale_Patch_Hook{ REL::RelocationID(37013, 37943), REL::Relocate(0x1A, 0x51) };	
	static REL::Relocation<std::uintptr_t> armorRating1(REL::RelocationID(42842, 44014));
	static REL::Relocation<std::uintptr_t> armorRating2(REL::RelocationID(37605, 38558));

	bool InstallHooks();
	bool InstallBashMultHook();
}
