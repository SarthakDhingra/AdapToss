#include <iostream>
#include <sc2api/sc2_unit_filters.h>

#include "BasicSc2Bot.h"

using namespace sc2;

void BasicSc2Bot::OnGameStart() {
	scouting_system.Init(Observation(), Actions());

	return;
}

void BasicSc2Bot::OnStep() {
	//TryBuildSupplyDepot();
	//TryBuildBarracks();

	scouting_system.ScoutingStep();

	// If we have less than 24 supply used, do opener things.
	int food_used = Observation()->GetFoodUsed();
	if (InBasicOpener(food_used))
	{
		TryBuildAdept();
		TryBuildWallPylon();
		TryBuildGeyser();
		TryBuildExpo();
		TryBuildCyber();
		TryBuildFirstGateway();
		TryBuildCliffPylon();
	}
	return;
}

bool BasicSc2Bot::TryBuildAdept()
{
	return false;
}

bool BasicSc2Bot::TryBuildWallPylon()
{
	return false;
}

bool BasicSc2Bot::TryBuildGeyser()
{
	return false;
}

bool BasicSc2Bot::TryBuildExpo()
{
	return false;
}

bool BasicSc2Bot::TryBuildCyber()
{
	return false;
}

bool BasicSc2Bot::TryBuildFirstGateway()
{
	//if (observation->)
	return false;
}

bool BasicSc2Bot::TryBuildCliffPylon()
{
	const ObservationInterface* observation = Observation();

	// builds our first pylon at 14 supply.
	if (observation->GetFoodCap() < 16 && observation->GetFoodUsed() > 13)
	{
		return TryBuildStructure(ABILITY_ID::BUILD_PYLON);
	}
	return false;
}

bool BasicSc2Bot::InBasicOpener(int food_used) const
{
	if (food_used < 24)
	{
		return true;
	}
	return false;
}

size_t BasicSc2Bot::CountUnitType(UNIT_TYPEID unit_type) {
	return Observation()->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
}

bool BasicSc2Bot::TryBuildBarracks() {
	const ObservationInterface* observation = Observation();

	if (CountUnitType(UNIT_TYPEID::TERRAN_SUPPLYDEPOT) < 1) {
		return false;
	}

	if (CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS) > 0) {
		return false;
	}

	return TryBuildStructure(ABILITY_ID::BUILD_BARRACKS);

}

const Unit* BasicSc2Bot::FindNearestMineralPatch(const Point2D& start) {
	Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
	float distance = std::numeric_limits<float>::max();
	const Unit* target = nullptr;
	for (const auto& u : units) {
		if (u->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD) {
			float d = DistanceSquared2D(u->pos, start);
			if (d < distance) {
				distance = d;
				target = u;
			}
		}
	}

	return target;

}

void BasicSc2Bot::OnUnitIdle(const Unit* unit) {
	switch (unit->unit_type.ToType()) {
	case UNIT_TYPEID::TERRAN_COMMANDCENTER: {
		Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_SCV);
		break;
	}
	case UNIT_TYPEID::PROTOSS_NEXUS: {			// trains workers until full.
		if (Observation()->GetMinerals() >= 50 && unit->assigned_harvesters < 22
			&& Observation()->GetFoodWorkers() < 80 && Observation()->GetFoodUsed() < Observation()->GetFoodCap())
		{
			Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_PROBE);
		}
	}
	case UNIT_TYPEID::PROTOSS_PROBE: {
		const Unit* mineral_target = FindNearestMineralPatch(unit->pos);
		if (!mineral_target) {
			break;
		}
		Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
		break;
	}
	case UNIT_TYPEID::TERRAN_SCV: {
		const Unit* mineral_target = FindNearestMineralPatch(unit->pos);
		if (!mineral_target) {
			break;
		}
		Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
		break;
	}
	case UNIT_TYPEID::TERRAN_BARRACKS: {
		Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_MARINE);
		break;
	}
	case UNIT_TYPEID::TERRAN_MARINE: {
		const GameInfo& game_info = Observation()->GetGameInfo();
		Actions()->UnitCommand(unit, ABILITY_ID::ATTACK_ATTACK, game_info.enemy_start_locations.front());
		break;
	}
	default: {
		break;
	}
	}
}

bool BasicSc2Bot::TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type) {
	const ObservationInterface* observation = Observation();

	// If a unit already is building a supply structure of this type, do nothing.
	// Also get an scv to build the structure.
	const Unit* unit_to_build = nullptr;
	Units units = observation->GetUnits(Unit::Alliance::Self);
	for (const auto& unit : units) {
		for (const auto& order : unit->orders) {
			if (order.ability_id == ability_type_for_structure) {
				return false;
			}
		}

		if (unit->unit_type == unit_type) {
			unit_to_build = unit;
		}
	}

	// if no  unit assigned return false (prevents reading nullptr exception)
	if (!unit_to_build) {
		return false;
	}

	float rx = GetRandomScalar();
	float ry = GetRandomScalar();

	Actions()->UnitCommand(unit_to_build,
		ability_type_for_structure,
		Point2D(unit_to_build->pos.x + rx * 15.0f, unit_to_build->pos.y + ry * 15.0f));
	return true;

}

bool BasicSc2Bot::TryBuildSupplyDepot() {
	const ObservationInterface* observation = Observation();

	// If we are not supply capped, don't build a supply depot.
	if (observation->GetFoodUsed() <= observation->GetFoodCap() - 2)
		return false;

	// Try and build a depot. Find a random SCV and give it the order.
	return TryBuildStructure(ABILITY_ID::BUILD_SUPPLYDEPOT);

}
