#include <iostream>
#include <sc2api/sc2_unit_filters.h>
#include <vector>
#include "ScoutingSystem.h"

using namespace sc2;

void ScoutingSystem::Init(const ObservationInterface* obs, ActionInterface* act, std::vector<Point3D> locs) {
	// Scouting system needs to be initialized on game start rather than at construction
	observation = obs;
	actions = act;
	exp_loc = locs;
	enemy_race = observation->GetGameInfo().player_info[1].race_requested;

	InitScoutingData();
}

void ScoutingSystem::InitScoutingData() {	
	// TODO - tune early scouting values so they meaningfully represent the game state

	const GameInfo& game_info = observation->GetGameInfo();
	//range is 6, get all positions that are 6 away, diagonals not considered for ease
	for (float x = game_info.playable_min.x; x < game_info.playable_max.x; x += 6.0f){
		for (float y = game_info.playable_min.y; y < game_info.playable_max.y; y += 6.0f){
			scout_locs.push(Point2D(x,y));
		}
	}
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
	// early out if system is turned off
	if (!toggle_on) {
		return;
	}
	
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
	
	Units units = observation->GetUnits(Unit::Alliance::Self, IsUnit(scout_type));
	for (const auto& unit : units) {
		scout = unit;
		return;
	}
	return;
}

const Unit* ScoutingSystem::GetScout() const
{
	return scout;
}

void ScoutingSystem::SendScout(const Unit * unit, bool dom_mode) {
	//set up so we can take a unit as a parameter in optionally
	auto scout_unit = scout;
	if (unit){
		scout_unit = unit;
	}
	// Return if no scout is set
	if (!scout_unit) {
		return;
	}

	const GameInfo& game_info = observation->GetGameInfo();

	// TODO: is there some way to combine GetUnit calls?
	int num_gateways = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::PROTOSS_GATEWAY)).size();
	int num_warpgates = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::PROTOSS_WARPGATE)).size();
	
	// If no gateways + warpgates, and enemy isn't zerg, don't scout yet
	if (num_gateways + num_warpgates == 0) {
		if (enemy_race != Race::Zerg) {
			return;
		}
	}

	//move randomly
	if (scout_unit->unit_type.ToType() == UNIT_TYPEID::PROTOSS_DARKTEMPLAR && dom_mode){
		//taken from bot_examples.cc

		//get played region size
		float playable_w = game_info.playable_max.x - game_info.playable_min.x;
    	float playable_h = game_info.playable_max.y - game_info.playable_min.y;
		
		//go some fraction of the playable space
		auto t_x = playable_w * GetRandomFraction() + game_info.playable_min.x;
		auto t_y = playable_h * GetRandomFraction() + game_info.playable_min.y;
		actions->UnitCommand(scout_unit, ABILITY_ID::MOVE_MOVE, Point2D(t_x,t_y));
	}
	//do a coordinated sweep around the map
	else if (scout_unit->unit_type.ToType() == UNIT_TYPEID::PROTOSS_VOIDRAY && dom_mode) {
		auto top = scout_locs.front();
		actions->UnitCommand(scout_unit, ABILITY_ID::MOVE_MOVE, scout_locs.front());
		scout_locs.pop();
		scout_locs.push(top);
	}
	else if (tasks.count(scout_unit) == 0 || Distance2D(scout_unit->pos, exp_loc[tasks[scout_unit]]) < 6.0f){
		//check if our scout already has a task or if close enough to reassign
		//send scout
		actions->UnitCommand(scout_unit, ABILITY_ID::MOVE_MOVE, exp_loc[pos]);
		//when giving a task store that in the map
		tasks[scout_unit] = pos;
		//increment to next expansion location
		pos = (pos + 1) % exp_loc.size();
		
	}

	return;
}

void ScoutingSystem::ScoutEarlyRush() {
	int supply = observation->GetFoodUsed();
	int num_gas = 0;

	if (enemy_race == Race::Zerg) {
		num_gas = observation->GetUnits(Unit::Alliance::Enemy, IsUnit(UNIT_TYPEID::ZERG_EXTRACTOR)).size();
		
		// Check for early spawn pool
		int num_pools = observation->GetUnits(Unit::Alliance::Enemy, IsUnit(UNIT_TYPEID::ZERG_SPAWNINGPOOL)).size();
		
		if (num_pools > 0 && supply < early_scouting_thresholds["spawning_pool"]) {
			scouting_data["early_rush"] = true;
		}
	}
	else if (enemy_race == Race::Terran) {
		num_gas = observation->GetUnits(Unit::Alliance::Enemy, IsUnit(UNIT_TYPEID::TERRAN_REFINERY)).size();
		
		// Check for lots of barracks
		int num_barracks = observation->GetUnits(Unit::Alliance::Enemy, IsUnit(UNIT_TYPEID::TERRAN_BARRACKS)).size();

		if (num_barracks > 2 && supply > early_scouting_thresholds["barracks"]) {
			scouting_data["early_rush"] = true;
		}
	}
	else {
		num_gas = observation->GetUnits(Unit::Alliance::Enemy, IsUnit(UNIT_TYPEID::PROTOSS_ASSIMILATOR)).size();
		
		// Check for early forge
		int num_forges = observation->GetUnits(Unit::Alliance::Enemy, IsUnit(UNIT_TYPEID::PROTOSS_FORGE)).size();

		if (num_forges > 0 && supply < early_scouting_thresholds["forge"]) {
			scouting_data["early_rush"] = true;
		}
	}

	// Check for early gas
	if (num_gas > 1 && supply < early_scouting_thresholds["gas"]) {
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
