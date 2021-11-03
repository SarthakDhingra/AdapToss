#include <iostream>
#include <sc2api/sc2_unit_filters.h>

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
	// Return if no scout is set
	if (!scout) {
		return;
	}

	const GameInfo& game_info = observation->GetGameInfo();

	// TODO: is there some way to combine GetUnit calls?
	int num_gateways = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::PROTOSS_GATEWAY)).size();
	int num_warpgates = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::PROTOSS_WARPGATE)).size();
	
	// If no gateways + warpgates, and enemy isn't zerg, don't scout yet
	if (num_gateways + num_warpgates == 0) {
		// hard coded that 1 is enemy
		if (game_info.player_info[1].race_requested != Race::Zerg) {
			std::cout << "error" << std::endl;
			return;
		}
	}

	// Send scout to enemy base
	actions->UnitCommand(scout, ABILITY_ID::MOVE_MOVE, game_info.enemy_start_locations.front());

	return;
}