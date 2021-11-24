#include <iostream>
#include <sc2api/sc2_unit_filters.h>

#include "BasicSc2Bot.h"

using namespace sc2;

void BasicSc2Bot::OnGameStart() {
	scouting_system.Init(Observation(), Actions());

	return;
}

void BasicSc2Bot::OnStep() {

	scouting_system.ScoutingStep();

	// If we have less than 24 supply used, do opener things.
	int food_used = Observation()->GetFoodUsed();
	if (InBasicOpener(food_used))
	{
		TryBuildWallPylon();
		TryBuildGeyser();
		TryBuildExpo();
		TryBuildCyber();
		TryBuildFirstGateway();
		TryBuildCliffPylon();
	}
	return;
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
	const ObservationInterface* observation = Observation();
	if (CountUnitType(UNIT_TYPEID::PROTOSS_GATEWAY) > 0 && CountUnitType(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) == 0)
	{
		return TryBuildStructure(ABILITY_ID::BUILD_CYBERNETICSCORE, UNIT_TYPEID::PROTOSS_PROBE, UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
	}

	return false;
}

bool BasicSc2Bot::TryBuildFirstGateway()
{
	const ObservationInterface* observation = Observation();
	if (observation->GetFoodUsed() > 14 && CountUnitType(UNIT_TYPEID::PROTOSS_GATEWAY) == 0)
	{
		return TryBuildStructure(ABILITY_ID::BUILD_GATEWAY, UNIT_TYPEID::PROTOSS_PROBE, UNIT_TYPEID::PROTOSS_GATEWAY);
	}

	return false;
}

bool BasicSc2Bot::TryBuildCliffPylon()
{
	const ObservationInterface* observation = Observation();

	// builds our first pylon at 14 supply.
	if (observation->GetFoodCap() < 16 && observation->GetFoodUsed() > 13)
	{
		return TryBuildStructure(ABILITY_ID::BUILD_PYLON, UNIT_TYPEID::PROTOSS_PROBE, UNIT_TYPEID::PROTOSS_PYLON);
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
	case UNIT_TYPEID::PROTOSS_NEXUS: {			// trains workers until full.
		if (Observation()->GetMinerals() >= 50 && unit->assigned_harvesters < unit->ideal_harvesters
			&& Observation()->GetFoodWorkers() < 80 && Observation()->GetFoodUsed() < Observation()->GetFoodCap())
		{
			Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_PROBE);
		}
	}

	case UNIT_TYPEID::PROTOSS_GATEWAY: {
		if (CountUnitType(UNIT_TYPEID::PROTOSS_ADEPT) < 2 && CountUnitType(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE))
		{
			Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_ADEPT);
			break;
		}
	}
	case UNIT_TYPEID::PROTOSS_CYBERNETICSCORE: {
		Actions()->UnitCommand(unit, ABILITY_ID::RESEARCH_WARPGATE);
		break;
	}
	case UNIT_TYPEID::PROTOSS_PROBE: {
		const Unit* mineral_target = FindNearestMineralPatch(unit->pos);
		if (!mineral_target) {
			break;
		}
		Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
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


// version for building only one building.
bool BasicSc2Bot::TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type, UNIT_TYPEID structure_type) {
	const ObservationInterface* observation = Observation();

	// If a unit already is building a supply structure of this type, do nothing.
	// Also get a probe to build the structure.
	const Unit* unit_to_build = nullptr;
	Units units = observation->GetUnits(Unit::Alliance::Self);
	for (const auto& unit : units)
	{	// checks if we are already building a structure of this type.
		if (unit->unit_type == structure_type && unit->build_progress < 1.0)
		{
			return false;
		}
	}
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

	if (structure_type != UNIT_TYPEID::PROTOSS_PYLON)
	{
		for (PowerSource powersource : observation->GetPowerSources())
		{
			Actions()->UnitCommand(unit_to_build,
				ability_type_for_structure,
				Point2D(powersource.position.x + rx * powersource.radius, powersource.position.y + ry * powersource.radius));
			return true;
		}
	}
	else
	{
		Actions()->UnitCommand(unit_to_build,
			ability_type_for_structure,
			Point2D(unit_to_build->pos.x + rx * 15.0f, unit_to_build->pos.y + ry * 15.0f));
		return true;
	}

	return false;

}