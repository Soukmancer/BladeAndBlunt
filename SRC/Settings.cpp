#include "Settings.h"
#include <SimpleIni.h>
#include <sstream>
#include "Cache.h"

Settings* Settings::GetSingleton()
{
	static Settings settings;
	return &settings;
}

void Settings::LoadSettings()
{
	CSimpleIniA ini;
	ini.SetUnicode();
	ini.LoadFile(R"(.\Data\SKSE\Plugins\BladeAndBlunt.ini)");

	armorScalingEnabled = ini.GetBoolValue("", "bArmorRatingScalingEnabled", true);

}

void Settings::SetGlobalsAndGameSettings() 
{
		//Set fMaxArmorRating game setting
	auto gameSettings = RE::GameSettingCollection::GetSingleton();
	auto maxRatingSetting = gameSettings->GetSetting("fMaxArmorRating");

	if (armorScalingEnabled) {
		logger::info("Setting max armor rating to 90");
		maxRatingSetting->data.f = 90.0f;
	} else {
		logger::info("Setting max armor rating to 75");
		maxRatingSetting->data.f = 75.0f;
	}
}

