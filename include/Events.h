#pragma once
#include <Conditions.h>
#include <RecentHitEventData.h>

class OnHitEventHandler : public RE::BSTEventSink<RE::TESHitEvent>
{
public:
	std::multimap<std::uint32_t, RecentHitEventData> recentGeneralHits;

	static OnHitEventHandler* GetSingleton()
	{
		static OnHitEventHandler singleton;
		return &singleton;
	}

	//TODO-Temp fix until I can upgrade clib versions
	std::uint32_t GetDurationOfApplicationRunTime()
	{
		REL::Relocation<std::uint32_t*> runtime{ RELOCATION_ID(523662, 410201) };
		return *runtime;
	}

	RE::BSEventNotifyControl ProcessEvent(const RE::TESHitEvent* a_event, [[maybe_unused]] RE::BSTEventSource<RE::TESHitEvent>* a_eventSource) override
	{
		if (!a_event || !a_event->target || !a_event->cause) {
			return RE::BSEventNotifyControl::kContinue;
		}

		auto causeActor = a_event->cause->As<RE::Actor>();
		auto targetActor = a_event->target->As<RE::Actor>();
	
		if (causeActor && targetActor && targetActor->IsPlayerRef() && !causeActor->IsPlayerRef()) {
			auto applicationRuntime = GetDurationOfApplicationRunTime();

			bool skipEvent = ShouldSkipHitEvent(causeActor, targetActor, applicationRuntime);	//Filters out dupe events

			if (!skipEvent) {			
				
				//Something is effed with power attacks. The source isnt coming through and casting as a weapon and the hit flags are empty
				//We can work around it like this
				bool powerAttackMelee = false;
				if (a_event->flags.any(RE::TESHitEvent::Flag::kPowerAttack) || Conditions::IsPowerAttacking(causeActor)) {
					bool rightIsMeleeWeapon = false;
					if (auto rightHandForm = causeActor->GetEquippedObject(false)) {
						if (rightHandForm->IsWeapon() && rightHandForm->As<RE::TESObjectWEAP>()->IsMelee()) {
							rightIsMeleeWeapon = true;
						}
					}

					bool LeftIsMeleeWeaponOrNone = false;
					auto leftHandForm = causeActor->GetEquippedObject(true);
					if (!leftHandForm || (leftHandForm->IsWeapon() && leftHandForm->As<RE::TESObjectWEAP>()->IsMelee()) || leftHandForm->IsArmor()) {
						LeftIsMeleeWeaponOrNone = true;
					}

					if (rightIsMeleeWeapon && LeftIsMeleeWeaponOrNone) {
						powerAttackMelee = true;
					}
				}

								//Shield Stagger
			recentGeneralHits.insert(std::make_pair(applicationRuntime, RecentHitEventData(targetActor, causeActor, applicationRuntime)));
			}
		}
		return RE::BSEventNotifyControl::kContinue;
	}

	static void ProcessHitEventForParry(RE::Actor* target, RE::Actor* aggressor)
	{
		auto settings = Settings::GetSingleton();
		if (Conditions::PlayerHasActiveMagicEffect(settings->MAG_ParryWindowEffect)) {
			Conditions::ApplySpell(target, aggressor, settings->MAGParryStaggerSpell);
		}
	}

	static void ProcessHitEventForBlockStagger(RE::Actor* target, RE::Actor* aggressor)
	{
		auto settings = Settings::GetSingleton();
		if (target->HasPerk(settings->BlockStaggerPerk)) {
			Conditions::ApplySpell(target, aggressor, settings->MAGBlockStaggerSpell2);
		}
		else {
			Conditions::ApplySpell(target, aggressor, settings->MAGBlockStaggerSpell);
		}
	}

	bool ShouldSkipHitEvent(RE::Actor* causeActor, RE::Actor* targetActor, std::uint32_t runTime)
	{
		bool skipEvent = false;
		
		auto matchedHits = recentGeneralHits.equal_range(runTime);
		for (auto it = matchedHits.first; it != matchedHits.second; ++it) {
			if (it->second.cause == causeActor && it->second.target == targetActor) {
				skipEvent = true;
				break;
			}
		}

		auto upper = recentGeneralHits.lower_bound(runTime);
		auto it = recentGeneralHits.begin();
		while (it != upper) {
			it = recentGeneralHits.erase(it);
		}

		return skipEvent;
	}

	static void Register()
	{
		RE::ScriptEventSourceHolder* eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
		eventHolder->AddEventSink(OnHitEventHandler::GetSingleton());
	}
};

class AnimationGraphEventHandler :
    public RE::BSTEventSink<RE::BSAnimationGraphEvent>,
    public RE::BSTEventSink<RE::TESObjectLoadedEvent>,
    public RE::BSTEventSink<RE::TESSwitchRaceCompleteEvent>

{
public:

    static AnimationGraphEventHandler* GetSingleton()
    {
        static AnimationGraphEventHandler singleton;
        return &singleton;
    }

    const char* jumpAnimEventString = "JumpUp";

    //Anims
    RE::BSEventNotifyControl ProcessEvent(const RE::BSAnimationGraphEvent* a_event, [[maybe_unused]] RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource) override
    {
        if (!a_event) {
            return RE::BSEventNotifyControl::kContinue;
        }

        if (!a_event->tag.empty() && a_event->holder && a_event->holder->As<RE::Actor>()) {
            if (std::strcmp(a_event->tag.c_str(), jumpAnimEventString) == 0) {

                HandleJumpAnim();
            }
        }
        
        return RE::BSEventNotifyControl::kContinue;
    }

    void HandleJumpAnim()
    {
        auto settings = Settings::GetSingleton();
        auto player   = RE::PlayerCharacter::GetSingleton();
        if (!player->IsGodMode()) {
            Conditions::ApplySpell(player, player, settings->jumpSpell);
        }
    }

    //Object load
    RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectLoadedEvent* a_event, [[maybe_unused]] RE::BSTEventSource<RE::TESObjectLoadedEvent>* a_eventSource) override
    {
        if (!a_event) {
            return RE::BSEventNotifyControl::kContinue;
        }
        
        const auto actor = RE::TESForm::LookupByID<RE::Actor>(a_event->formID);
        if (!actor || !actor->IsPlayerRef()) {
            return RE::BSEventNotifyControl::kContinue;
        }

        //Register for anim event
        actor->AddAnimationGraphEventSink(AnimationGraphEventHandler::GetSingleton());

        return RE::BSEventNotifyControl::kContinue;
    }

    // Race Switch
    RE::BSEventNotifyControl ProcessEvent(const RE::TESSwitchRaceCompleteEvent* a_event, [[maybe_unused]] RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>* a_eventSource) override
    {
        if (!a_event) {
            return RE::BSEventNotifyControl::kContinue;
        }

        const auto actor = a_event->subject->As<RE::Actor>();
        if (!actor || !actor->IsPlayerRef()) {
            return RE::BSEventNotifyControl::kContinue;
        }

        // Register for anim event
        actor->AddAnimationGraphEventSink(AnimationGraphEventHandler::GetSingleton());

        return RE::BSEventNotifyControl::kContinue;
    }

    static void Register()
    {

        //Register for load event, then in the load event register for anims
        RE::ScriptEventSourceHolder* eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
        eventHolder->AddEventSink<RE::TESObjectLoadedEvent>(GetSingleton());
        eventHolder->AddEventSink<RE::TESSwitchRaceCompleteEvent>(GetSingleton());
    }
};

class WeaponFireHandler
{
public:
	static void WeaponFire(RE::TESObjectWEAP* a_weapon, RE::TESObjectREFR* a_source, RE::TESAmmo* a_ammo, RE::EnchantmentItem* a_ammoEnchantment, RE::AlchemyItem* a_poison)
	{
		_Weapon_Fire(a_weapon, a_source, a_ammo, a_ammoEnchantment, a_poison);

		if (!a_source) {
			return;
		}

		auto source = a_source->As<RE::Actor>();

		if (source->IsPlayerRef() && a_weapon->IsCrossbow()) {
			Conditions::ApplySpell(source, source,Settings::GetSingleton()->MAGCrossbowStaminaDrainSpell);
		}
	}
	inline static REL::Relocation<decltype(WeaponFire)> _Weapon_Fire;
};

