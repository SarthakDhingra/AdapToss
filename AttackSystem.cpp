#include <iostream>
#include <sc2api/sc2_unit_filters.h>

#include "AttackSystem.h"

using namespace sc2;

void AttackSystem::Init(const ObservationInterface* obs, ActionInterface* act) {
	// Attacking system needs to be initialized on game start rather than at construction
	observation = obs;
	actions = act;

	target = observation->GetGameInfo().enemy_start_locations.front();
}

void AttackSystem::AttackStep() {
	SetAttackUnits();
	SendAttackUnits();
}

void AttackSystem::SetAttackUnits() {
	// Offense should be filled with all dark templar available for now
	if (attack_units.size() != CountUnitType(UNIT_TYPEID::PROTOSS_DARKTEMPLAR)) {

		attack_units.clear();

		Units units = observation->GetUnits(Unit::Alliance::Self);
		for (const auto& unit : units) {
			if (unit->unit_type.ToType() == UNIT_TYPEID::PROTOSS_DARKTEMPLAR) {
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

	// Send attack units to target
	for (const auto& unit : attack_units) {
		actions->UnitCommand(unit, ABILITY_ID::ATTACK, target);
	}

	return;
}

void AttackSystem::SetTarget() {
	// Adjust target for attack units to try and attack

	return;
}


size_t AttackSystem::CountUnitType(UNIT_TYPEID unit_type) {
	return observation->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
}
