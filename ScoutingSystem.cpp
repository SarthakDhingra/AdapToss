#include <iostream>

#include "ScoutingSystem.h"

using namespace sc2;

void ScoutingSystem::Initialize(const ObservationInterface* obs, ActionInterface* act) {
	// Scouting system needs to be initialized on game start rather than at construction
	observation = obs;
	actions = act;
}

void ScoutingSystem::ScoutingStep() {
	SetScout();
	SendScout();
	
	return;
}

void ScoutingSystem::SetScout() {
	// Early out if there's currently a scout
	if (scout && scout->is_alive) {
		return;
	}
	
	Units units = observation->GetUnits(Unit::Alliance::Self);

	for (const auto& unit : units) {
		if (unit->unit_type.ToType() == scout_type) {
			scout = unit;
			return;
		}
	}
	return;
}

void ScoutingSystem::SendScout() {
	// return if no scout is set
	if (!scout) {
		return;
	}

	// Send scout to enemy base
	const GameInfo& game_info = observation->GetGameInfo();
	actions->UnitCommand(scout, ABILITY_ID::MOVE_MOVE, game_info.enemy_start_locations.front());

	return;
}