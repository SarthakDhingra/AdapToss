#include <iostream>
#include <sc2api/sc2_unit_filters.h>

#include "DefenseSystem.h"

using namespace sc2;

void DefenseSystem::Init(const ObservationInterface* obs, ActionInterface* act) {
	// Scouting system needs to be initialized on game start rather than at construction
	observation = obs;
	actions = act;
}

void DefenseSystem::DefenseStep() {
	SetDefense();
	SendDefense();
}

void  DefenseSystem::SetDefense() {

	// later should pull this from map of types
	UNIT_TYPEID adept_type = UNIT_TYPEID::PROTOSS_ADEPT;

	// defense should be filled with all adepts available
	if (defense.size() != CountUnitType(UNIT_TYPEID::PROTOSS_ADEPT)) {

		//clear out dfense
		defense.clear();

		// push all adepts to defense vector
		Units units = observation->GetUnits(Unit::Alliance::Self);
		for (const auto& unit : units) {
			if (unit->unit_type.ToType() == adept_type) {
				defense.push_back(unit);
			}
		}

	}

}

void DefenseSystem::SendDefense() {

	// get all enemies
	Units enemies = observation->GetUnits(Unit::Alliance::Enemy);
	const GameInfo& game_info = observation->GetGameInfo();

	// this value needs to be tuned
	float distance = 400;

	// TODO: If a large number of enemies are amassing and are closeish to the base, we can create more defense units maybe? 
	for (const auto& e : enemies) {
		float d = DistanceSquared2D(e->pos, game_info.start_locations.back());

		// Attack enemy with all defenders
		if (d < distance) {
			for (const auto& d : defense) {
				actions->UnitCommand(d, ABILITY_ID::ATTACK, e->pos);
			}
		}
	}

}

size_t DefenseSystem::CountUnitType(UNIT_TYPEID unit_type) {
	return observation->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
}