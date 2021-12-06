#include <iostream>
#include <sc2api/sc2_unit_filters.h>

#include "AttackSystem.h"

using namespace sc2;

void AttackSystem::Init(const ObservationInterface* obs, ActionInterface* act, std::vector<Point3D> locs){
	// Attacking system needs to be initialized on game start rather than at construction
	observation = obs;
	actions = act;
	exp_locs = locs;
}

void AttackSystem::AttackStep() {
	SetAttackUnits();
	SendAttackUnits();
}

void AttackSystem::SetAttackUnits() {
	// Offense should be filled with all dark templar and void rays available for now
	if (attack_units.size() != CountUnitType(UNIT_TYPEID::PROTOSS_DARKTEMPLAR) + CountUnitType(UNIT_TYPEID::PROTOSS_VOIDRAY)) {

		attack_units.clear();

		Units units = observation->GetUnits(Unit::Alliance::Self);
		for (const auto& unit : units) {
			if (unit->unit_type.ToType() == UNIT_TYPEID::PROTOSS_DARKTEMPLAR || unit->unit_type.ToType() == UNIT_TYPEID::PROTOSS_VOIDRAY) {
				attack_units.push_back(unit);
			}
		}
	}
}

void AttackSystem::SendAttackUnits() {
	// Send all attack units to enemy base location
	
	// Return if no attack units are set
	if (attack_units.empty()) {
		return;
	}

	// Send attack units to target, if no target, let it go idle so it is sent into scouting mode
	for (const auto& unit : attack_units) {
		auto target = GetTarget(unit);
		if (target.x != observation->GetGameInfo().width + 10){
			actions->UnitCommand(unit, ABILITY_ID::ATTACK, target);
		}
	}

	return;
}

Point3D AttackSystem::GetTarget(const Unit * unit) {
	// Get closest enemy to this unit for it to attack
	Units enemies = observation->GetUnits(Unit::Alliance::Enemy);
 	if (enemies.size() > 0){
 		size_t closest = 0;
 		float dist = std::numeric_limits<float>::max();
 		for (int i = 0; i < enemies.size(); i++){
			if (unit->unit_type != UNIT_TYPEID::PROTOSS_VOIDRAY && enemies[i]->is_flying){
				continue;
			}
 			auto d = Distance3D(unit->pos,enemies[i]->pos);
 			if (d < dist){
 				dist = d;
 				closest = i;
 			}
 		}
 		return enemies[closest]->pos;
 	}


	return Point3D(observation->GetGameInfo().width + 10, 0, 0);
}

size_t AttackSystem::CountUnitType(UNIT_TYPEID unit_type) {
	return observation->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
}
