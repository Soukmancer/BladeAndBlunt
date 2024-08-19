#include "UpdateManager.h"
#include "Events.h"
#include "patches/ArmorRatingScaling.h"
#include "patches/MiscPatches.h"

namespace Hooks
{
	bool InstallHooks()
	{
		if (!UpdateManager::Install()) { return false; }
		if (!MiscPatches::InstallScalePatch()) { return false; }
	
		auto runtime = REL::Module::GetRuntime();
		if (Settings::GetSingleton()->armorScalingEnabled) {
			if (runtime == REL::Module::Runtime::AE) {
				logger::info("Installing ar hook AE");
				ArmorRatingScaling::InstallArmorRatingHookAE();
			} else {
				logger::info("Installing ar hook SE");
				ArmorRatingScaling::InstallArmorRatingHookSE();	
			}
			logger::info("Installed ar hook");
		}

			return true;
	}

}
