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
		CheckHarvesterStatus();
	}
	return;
}

bool BasicSc2Bot::TryBuildWallPylon()
{
	const ObservationInterface* observation = Observation();

	// builds our first pylon at 14 supply.
	if (observation->GetFoodCap() > 16 && observation->GetFoodUsed() > 19)
	{
		return TryBuildStructure(ABILITY_ID::BUILD_PYLON, UNIT_TYPEID::PROTOSS_PROBE, UNIT_TYPEID::PROTOSS_PYLON);
	}

	return false;
}

bool BasicSc2Bot::TryBuildGeyser()
{
	if (Observation()->GetFoodWorkers() > 15 && CountUnitType(UNIT_TYPEID::PROTOSS_ASSIMILATOR) < 2)
	{
		return TryBuildStructure(ABILITY_ID::BUILD_ASSIMILATOR, UNIT_TYPEID::PROTOSS_PROBE, UNIT_TYPEID::PROTOSS_ASSIMILATOR);
	}
	return false;
}

bool BasicSc2Bot::TryBuildExpo()
{
	// todo: code this. It isn't going to be simple lol :p
	const ObservationInterface* observation = Observation();
	int nexus_count = CountUnitType(UNIT_TYPEID::PROTOSS_NEXUS);

	// builds a nexus whenever we have no nexus, or if the food to nexus ratio is too high
	if (nexus_count == 0 || (float)observation->GetFoodUsed() / (float)nexus_count > 16 + (3 * nexus_count))
	{
		return TryBuildStructure(ABILITY_ID::BUILD_NEXUS, UNIT_TYPEID::PROTOSS_PROBE, UNIT_TYPEID::PROTOSS_NEXUS);
	}
	return false;
}

bool BasicSc2Bot::TryBuildCyber()
{
	if (CountUnitType(UNIT_TYPEID::PROTOSS_GATEWAY) > 0 && CountUnitType(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) == 0)
	{
		return TryBuildStructure(ABILITY_ID::BUILD_CYBERNETICSCORE, UNIT_TYPEID::PROTOSS_PROBE, UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
	}

	return false;
}

bool BasicSc2Bot::TryBuildFirstGateway()
{
	if (Observation()->GetFoodUsed() > 14 && CountUnitType(UNIT_TYPEID::PROTOSS_GATEWAY) == 0)
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
	if (food_used < 30)
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

// assigns a probe to the geyser.
bool BasicSc2Bot::AssignProbeToGas(const Unit *geyser)
{
	const ObservationInterface* observation = Observation();
	// If a unit already is building a supply structure of this type, do nothing.
	// Also get an scv to build the structure.
	Units units_to_assign;
	Units units = observation->GetUnits(Unit::Alliance::Self);
	for (const auto& unit : units) {
		if (unit->unit_type == UNIT_TYPEID::PROTOSS_PROBE && (unit->orders.empty() || (unit->orders[0].ability_id == ABILITY_ID::SMART 
			&& observation->GetUnit(unit->orders[0].target_unit_tag)->unit_type != UNIT_TYPEID::PROTOSS_ASSIMILATOR))) {
			units_to_assign.push_back(unit);
			if (units_to_assign.size() >= geyser->ideal_harvesters - geyser->assigned_harvesters)
			{
				break;
			}
		}
	}

	// if no  unit assigned return false (prevents reading nullptr exception)
	if (units_to_assign.empty()) {
		return false;
	}

	Actions()->UnitCommand(units_to_assign, ABILITY_ID::SMART, geyser);

	return true;
}

bool BasicSc2Bot::CheckHarvesterStatus()
{
	Units units = Observation()->GetUnits(Unit::Alliance::Self);
	for (const auto& unit : units)
	{
		if (unit->unit_type == UNIT_TYPEID::PROTOSS_ASSIMILATOR)
		{
			if (unit->vespene_contents > 0 && unit->ideal_harvesters > unit->assigned_harvesters)
			{
				AssignProbeToGas(unit);
			}
		}
		else if (unit->unit_type == UNIT_TYPEID::PROTOSS_NEXUS)
		{
			if (unit->ideal_harvesters > unit->assigned_harvesters && unit->orders.empty())
			{
				Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_PROBE);
			}
		}
	}
	return true;
}

void BasicSc2Bot::OnUnitIdle(const Unit* unit) {
	switch (unit->unit_type.ToType()) {
	case UNIT_TYPEID::PROTOSS_NEXUS: {			// trains workers until full.
		// note, we need to update unit->assigned_harvesters because currently it counts scouting probes and dead probes.
		if (unit->assigned_harvesters < (unit->ideal_harvesters + 6) * CountUnitType(UNIT_TYPEID::PROTOSS_NEXUS))
		{
			//Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_PROBE);
		}
		break;
	}
	case UNIT_TYPEID::PROTOSS_GATEWAY: {
		if (CountUnitType(UNIT_TYPEID::PROTOSS_ADEPT) < 1 && CountUnitType(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE))
		{
			Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_ADEPT);
			break;
		}
		if (CountUnitType(UNIT_TYPEID::PROTOSS_ZEALOT) < 1)
		{
			Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_ZEALOT);
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

// returns the squared distance between two Units
float BasicSc2Bot::SqDist(const Unit* a, const Unit* b)
{
	float x, y;
	x = (a->pos.x - b->pos.x);
	x *= x;
	y = (a->pos.y - b->pos.y);
	y *= y;
	return x + y;
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

	if (structure_type == UNIT_TYPEID::PROTOSS_PYLON)
	{
		Actions()->UnitCommand(unit_to_build,
			ability_type_for_structure,
			Point2D(unit_to_build->pos.x + rx * 15.0f, unit_to_build->pos.y + ry * 15.0f));
		return true;

	}
	else if (structure_type == UNIT_TYPEID::PROTOSS_ASSIMILATOR)
	{
		Units poss_geysers = observation->GetUnits(Unit::Alliance(3));		// gets neutral units
		Units poss_nexi = observation->GetUnits(Unit::Alliance(1));			// gets friendly units
		Units nexi;
		for (const auto& poss_nex : poss_nexi)								// puts all friendly nexi into nexi. Used for finding nearby geysers.
		{
			if (poss_nex->unit_type == UNIT_TYPEID::PROTOSS_NEXUS)
			{
				nexi.push_back(poss_nex);
			}
		}
		std::vector<UNIT_TYPEID> gas_ids;
		gas_ids.push_back(UNIT_TYPEID::NEUTRAL_VESPENEGEYSER);
		gas_ids.push_back(UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER);
		gas_ids.push_back(UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER);
		gas_ids.push_back(UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER);
		gas_ids.push_back(UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER);
		gas_ids.push_back(UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER);
		for (const auto& poss_geyser : poss_geysers)
		{
			for (UNIT_TYPEID gas_id : gas_ids)				
			{
				if (poss_geyser->unit_type == gas_id)					// makes sure target is a geyser
				{
					bool close_to_nexus = false;
					for (const auto& poss_nex : nexi)
					{
						if (SqDist(poss_nex, poss_geyser) < 110)		// makes sure geyser is close enough to a nexus. Only loops once per gas_id
						{
							close_to_nexus = true;
						}
					}
					if (close_to_nexus)									// assigns the build task if we are near a nexus.
					{
						Actions()->UnitCommand(unit_to_build,
							ability_type_for_structure,
							poss_geyser);
						return true;
					}
				}
			}
		}
	}
	else if (structure_type == UNIT_TYPEID::PROTOSS_NEXUS)
	{
		Units poss_geysers = observation->GetUnits(Unit::Alliance(3));		// gets neutral units
		Units poss_nexi = observation->GetUnits(Unit::Alliance(1));			// gets friendly units
		Units nexi;
		float closest_expo_gas_dist = 1000000000000000000;
		float poss_distance;
		Tag closest_geyser_tag = 0;
		for (const auto& poss_nex : poss_nexi)								// puts all friendly nexi into nexi. Used for finding nearby geysers.
		{
			if (poss_nex->unit_type == UNIT_TYPEID::PROTOSS_NEXUS)
			{
				nexi.push_back(poss_nex);
			}
		}
		std::vector<UNIT_TYPEID> gas_ids;
		gas_ids.push_back(UNIT_TYPEID::NEUTRAL_VESPENEGEYSER);
		gas_ids.push_back(UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER);
		gas_ids.push_back(UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER);
		gas_ids.push_back(UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER);
		gas_ids.push_back(UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER);
		gas_ids.push_back(UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER);
		for (const auto& poss_geyser : poss_geysers)
		{
			for (UNIT_TYPEID gas_id : gas_ids)
			{
				if (poss_geyser->unit_type == gas_id)					// makes sure target is a geyser
				{
					for (const auto& poss_nex : nexi)
					{
						poss_distance = SqDist(poss_nex, poss_geyser);
						if (poss_distance > 110)		// makes sure geyser is close enough to a nexus. Only loops once per gas_id
						{
							if (poss_distance < closest_expo_gas_dist)
							{
								closest_expo_gas_dist = poss_distance;
								closest_geyser_tag = poss_geyser->tag;
							}
						}
					}
					if (nexi.empty()) // build near closest geyser to worker. handles no nexus situations
					{
						poss_distance = SqDist(unit_to_build, poss_geyser);
						if (poss_distance < closest_expo_gas_dist)
						{
							closest_expo_gas_dist = poss_distance;
							closest_geyser_tag = poss_geyser->tag;
						}
					}
				}
			}
		}

		if (closest_geyser_tag)
		{
			poss_distance = 100000000000000000;
			closest_expo_gas_dist = 10000000000;
			const Unit* geyser = observation->GetUnit(closest_geyser_tag);

			// gets nearest geyser to geyser
			for (const auto& poss_geyser : poss_geysers)
			{
				if (poss_geyser->tag != geyser->tag)
				{
					for (UNIT_TYPEID gas_id : gas_ids)
					{
						if (poss_geyser->unit_type == gas_id)					// makes sure target is a geyser
						{
							poss_distance = SqDist(geyser, poss_geyser);
							if (poss_distance < closest_expo_gas_dist)
							{
								closest_expo_gas_dist = poss_distance;
								closest_geyser_tag = poss_geyser->tag;
							}
							
						}
					}
				}
			}

			const Unit* geyser2 = observation->GetUnit(closest_geyser_tag);
			float midx = (geyser->pos.x + geyser2->pos.x) / 2.0;
			float midy = (geyser->pos.y + geyser2->pos.y) / 2.0;
			float x, y, dist1;
			float possx, possy;
			bool spot_found = false;
			while (!spot_found)
			{
				rx = GetRandomScalar();
				ry = GetRandomScalar();
				possx = midx + rx * 2.0f;
				possy = midy + ry * 2.0f;

				
				x = (possx - geyser->pos.x);
				x *= x;
				y = (possy - geyser->pos.y);
				y *= y;
				dist1 = x + y;

				x = (possx - geyser2->pos.x);
				x *= x;
				y = (possy - geyser2->pos.y);
				y *= y;
				dist1 -= (x + y);

				if (dist1 < 1.0 && dist1 > -1.0)
				{
					spot_found = true;
				}
				
			}


			Actions()->UnitCommand(unit_to_build,
				ability_type_for_structure,
				Point2D(possx, possy));
			return true;
		}
		return false;
	}
	else
	{
		for (const PowerSource powersource : observation->GetPowerSources())
		{
			Actions()->UnitCommand(unit_to_build,
				ability_type_for_structure,
				Point2D(powersource.position.x + rx * powersource.radius, powersource.position.y + ry * powersource.radius));
			return true;
		}
	}

	return false;

}