#include <iostream>
#include <sc2api/sc2_unit_filters.h>

#include "ScoutingSystem.h"

using namespace sc2;

void ScoutingSystem::Init(const ObservationInterface* obs, ActionInterface* act) {
	// Scouting system needs to be initialized on game start rather than at construction
	observation = obs;
	actions = act;

	enemy_race = observation->GetGameInfo().player_info[1].race_requested;

	InitScoutingData();
}

void ScoutingSystem::InitScoutingData() {	
	
	// TODO - tune early scouting values so they meaningfully represent the game state
	early_scouting_thresholds = {
		{"early_game", 30},
		{"gas", 10},
		{"spawning_pool", 10},
		{"barracks", 20},
		{"forge", 20},
	};

	scouting_data = {
		{"early_rush", false},
		{"detection", false},
	};
}

void ScoutingSystem::ScoutingStep() {
	SetScout();
	SendScout();

	// check for early rush in the early game
	if (observation->GetFoodUsed() < early_scouting_thresholds["early_game"]) {
		ScoutEarlyRush();
	}
	
	ScoutDetection();
	
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
		if (enemy_race != Race::Zerg) {
			std::cout << "error" << std::endl;
			return;
		}
	}

	// Send scout to enemy base
	actions->UnitCommand(scout, ABILITY_ID::MOVE_MOVE, game_info.enemy_start_locations.front());

	return;
}

void ScoutingSystem::ScoutEarlyRush() {
	int supply = observation->GetFoodUsed();
	int num_gas = 0;

	if (enemy_race == Race::Zerg) {
		num_gas = observation->GetUnits(Unit::Alliance::Enemy, IsUnit(UNIT_TYPEID::ZERG_EXTRACTOR)).size();
		
		// check for early spawn pool
		int num_pools = observation->GetUnits(Unit::Alliance::Enemy, IsUnit(UNIT_TYPEID::ZERG_SPAWNINGPOOL)).size();
		
		if (num_pools > 0 && supply < early_scouting_thresholds["spawning_pool"]) {
			scouting_data["early_rush"] = true;
		}
	}
	else if (enemy_race == Race::Terran) {
		num_gas = observation->GetUnits(Unit::Alliance::Enemy, IsUnit(UNIT_TYPEID::TERRAN_REFINERY)).size();
		
		// check for low barracks
		int num_barracks = observation->GetUnits(Unit::Alliance::Enemy, IsUnit(UNIT_TYPEID::TERRAN_BARRACKS)).size();

		if (num_barracks < 3 && supply > early_scouting_thresholds["barracks"]) {
			scouting_data["early_rush"] = true;
		}
	}
	else {
		num_gas = observation->GetUnits(Unit::Alliance::Enemy, IsUnit(UNIT_TYPEID::PROTOSS_ASSIMILATOR)).size();
		
		// check for early forge
		int num_forges = observation->GetUnits(Unit::Alliance::Enemy, IsUnit(UNIT_TYPEID::PROTOSS_FORGE)).size();

		if (num_forges > 0 && supply < early_scouting_thresholds["forge"]) {
			scouting_data["early_rush"] = true;
		}
	}

	// General case
	if (num_gas > 2 && supply < early_scouting_thresholds["gas"]) {
		scouting_data["early_rush"] = true;
	}
}

void ScoutingSystem::ScoutDetection() {
	UNIT_TYPEID detection_unit;

	if (enemy_race == Race::Zerg) {
		detection_unit = UNIT_TYPEID::ZERG_OVERSEER;
	}
	else if (enemy_race == Race::Terran) {
		detection_unit = UNIT_TYPEID::TERRAN_RAVEN;
	}
	else {
		detection_unit = UNIT_TYPEID::PROTOSS_OBSERVER;
	}

	int num_units = observation->GetUnits(Unit::Alliance::Enemy, IsUnit(detection_unit)).size();

	if (num_units > 0) {
		scouting_data["detection"] = true;
	}
}
