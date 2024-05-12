#include "hooks.h"
#include "settings.h"
#include "dodge.h"
#include "include/Utils.h"

namespace hooks
{

	static inline RE::BSFixedString cPtr_bashPowerStart = "bashPowerStart";
	/// <summary>
	/// Handle perilous attacking.
	/// </summary>
	/// <param name="a_actionData"></param>
	/// <returns>Whether the attack action is performed.</returns>

	void on_animation_event::ProcessEvent(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
	{
		if (!a_event->holder) {
			return;
		}
		std::string_view eventTag = a_event->tag.data();
		RE::Actor* actor = const_cast<RE::TESObjectREFR*>(a_event->holder)->As<RE::Actor>();
		switch (hash(eventTag.data(), eventTag.size())) {
		case "bashPowerStart"_h:
		case "PowerAttack_Start_end"_h:
		case "NextPowerAttackInitiate"_h:
			if ((dodge::GetSingleton()->GenerateRandomInt(0, 100)) <= 30) {
				dodge::GetSingleton()->react_to_attack(actor);
			}
			break;


		case "NextAttackInitiate"_h:
		case "MCO_AttackInitiate"_h:
		case "SCAR_ComboStart"_h:
			if ((dodge::GetSingleton()->GenerateRandomInt(0, 100)) <= 20) {
				dodge::GetSingleton()->react_to_attack(actor);
			}
			break;
		
		case "preHitFrame"_h:
        case "weaponSwing"_h:
		case "weaponLeftSwing"_h:
			if (!Utils::Actor::isHumanoid(actor) && (actor->AsActorState()->GetAttackState() == RE::ATTACK_STATE_ENUM::kDraw || actor->AsActorState()->GetAttackState() == RE::ATTACK_STATE_ENUM::kSwing)) {
				if ((Utils::Actor::isPowerAttacking(actor) == true) && (dodge::GetSingleton()->GenerateRandomInt(0, 100)) <= 30) {
					dodge::GetSingleton()->react_to_attack(actor);
				} else if ((dodge::GetSingleton()->GenerateRandomInt(0, 100)) <= 25) {
					dodge::GetSingleton()->react_to_attack(actor);
				}
			}
			break;
		
		case "BeginCastVoice"_h:
		case "BeginCastLeft"_h:
		case "BeginCastRight"_h:
			if ((dodge::GetSingleton()->GenerateRandomInt(0, 100)) <= 25) {
				dodge::GetSingleton()->react_to_attack(actor);
			}
			break;

		case "BowFullDrawn"_h:
		    if ((dodge::GetSingleton()->GenerateRandomInt(0, 100)) <= 5) {
				dodge::GetSingleton()->react_to_attack(actor);
			}
			break;


		case "MCO_DodgeInitiate"_h:
			dodge::GetSingleton()->set_dodge_phase(const_cast<RE::TESObjectREFR*>(a_event->holder)->As<RE::Actor>(), true);
			break;


		case "MCO_DodgeStop"_h:
		case "DodgeStop"_h:
			dodge::GetSingleton()->set_dodge_phase(const_cast<RE::TESObjectREFR*>(a_event->holder)->As<RE::Actor>(), false);
			break;
		}
	}

	EventResult on_animation_event::ProcessEvent_NPC(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
	{
		ProcessEvent(a_sink, a_event, a_eventSource);
		return _ProcessEvent_NPC(a_sink, a_event, a_eventSource);
	}

	EventResult on_animation_event::ProcessEvent_PC(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
	{
		ProcessEvent(a_sink, a_event, a_eventSource);
		return _ProcessEvent_PC(a_sink, a_event, a_eventSource);
	}

	ptr_CombatPath on_combatBehavior_backoff_createPath::create_path(RE::Actor* a_actor, RE::NiPoint3* a_newPos, float a3, int speed_ind)
	{
		switch (settings::iDodgeAI_Framework) {
		case 0:
			dodge::GetSingleton()->attempt_dodge(a_actor, &dodge_directions_tk_back);
			break;
		case 1:
			dodge::GetSingleton()->attempt_dodge(a_actor, &dodge_directions_dmco_back);
			break;
		}
		
		return _create_path(a_actor, a_newPos, a3, speed_ind);
	}

	ptr_CombatPath on_combatBehavior_circle_createPath::create_path(RE::Actor* a_actor, RE::NiPoint3* a_newPos, float a3, int speed_ind)
	{
		switch (settings::iDodgeAI_Framework) {
		case 0:
			dodge::GetSingleton()->attempt_dodge(a_actor, &dodge_directions_tk_horizontal);
			break;
		case 1:
			dodge::GetSingleton()->attempt_dodge(a_actor, &dodge_directions_dmco_horizontal);
			break;
		}
		

		return _create_path(a_actor, a_newPos, a3, speed_ind);
	}

	ptr_CombatPath on_combatBehavior_fallback_createPath::create_path(RE::Actor* a_actor, RE::NiPoint3* a_newPos, float a3, int speed_ind)
	{
		switch (settings::iDodgeAI_Framework) {
		case 0:
			dodge::GetSingleton()->attempt_dodge(a_actor, &dodge_directions_tk_back);
			break;
		case 1:
			dodge::GetSingleton()->attempt_dodge(a_actor, &dodge_directions_dmco_back);
			break;
		}
		
		return _create_path(a_actor, a_newPos, a3, speed_ind);
	}

	ptr_CombatPath on_combatBehavior_dodgethreat_createPath::create_path(RE::Actor* a_actor, RE::NiPoint3* a_newPos, float a3, int speed_ind)
	{
		switch (settings::iDodgeAI_Framework) {
		case 0:
			dodge::GetSingleton()->attempt_dodge(a_actor, &dodge_directions_tk_all, true);
			break;
		case 1:
			dodge::GetSingleton()->attempt_dodge(a_actor, &dodge_directions_dmco_all, true);
			break;
		}
		

		return _create_path(a_actor, a_newPos, a3, speed_ind);

	}
	


	inline void offset_NPC_rotation(RE::Actor* a_actor, float& a_angle) 
	{
		if (a_actor->IsPlayerRef()) {
			return;
		}
		float angleDelta = Utils::math::NormalRelativeAngle(a_angle - a_actor->data.angle.z);
	
		/* Dodge commitment */
		if (settings::bDodgeAI_Enable && dodge::GetSingleton()->get_is_dodging(a_actor)) {  
			angleDelta = 0;
		}
		a_angle = a_actor->data.angle.z + angleDelta;
	}

	
	void on_set_rotation::Actor_SetRotationX(RE::Actor* a_this, float a_angle)
	{
		offset_NPC_rotation(a_this, a_angle);
		_Actor_SetRotationX(a_this, a_angle);
	}

	void on_set_rotation::Actor_SetRotationZ(RE::Actor* a_this, float a_angle)
	{
		offset_NPC_rotation(a_this, a_angle);
		_Actor_SetRotationZ(a_this, a_angle);
	}

};
