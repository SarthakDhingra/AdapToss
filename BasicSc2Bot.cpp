#include <iostream>
#include <sc2api/sc2_unit_filters.h>

#include "BasicSc2Bot.h"

using namespace sc2;

void BasicSc2Bot::OnGameStart() {
	

	InitData();
	InitWarpInLocation();

	scouting_system.Init(Observation(), Actions(),exp_loc);
	defense_system.Init(Observation(), Actions());
	attack_system.Init(Observation(), Actions(), exp_loc);
	slander_system.Init(Observation(), Actions());

	return;
}

void BasicSc2Bot::OnStep() {
	scouting_system.ScoutingStep();
	defense_system.DefenseStep();
	attack_system.AttackStep();
	slander_system.SlanderStep();
	
	TryBuildPylon();
	CheckHarvesterStatus();

	// If we have less than a certain threshold of supply used, do opener things.
	if (InBasicOpener())
	{
		TryBuildGeyser();
		TryBuildExpo();
		TryBuildCyber();
		TryBuildGateway();
		TryBuildTwilight();
		TryBuildDarkshrine();
		TryBuildRoboticsFacility();
	}
	//if we have cleared out the map later in the game
	if (InDominationMode()){
		TryBuildStargate();
	}
	return;
}

void BasicSc2Bot::InitData() {
	//how close we get to the base with DTs
	approach_increment = 2.0;

	//get gas spots
	std::vector<UNIT_TYPEID> gas_ids;
	gas_ids.push_back(UNIT_TYPEID::NEUTRAL_VESPENEGEYSER);
	gas_ids.push_back(UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER);
	gas_ids.push_back(UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER);
	gas_ids.push_back(UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER);
	gas_ids.push_back(UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER);
	gas_ids.push_back(UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER);
	
	//find all neutral units (includes resources), go through them to find locations of gases
	//these cover the mains and the expansions
	Units poss_geysers = Observation()->GetUnits(Unit::Alliance::Neutral);		// gets neutral units
	for (const auto& poss_geyser : poss_geysers)
		{
			for (UNIT_TYPEID gas_id : gas_ids)
			{
				if (poss_geyser->unit_type == gas_id)					// makes sure target is a geyser
				{
					exp_loc.push_back(poss_geyser->pos);
				}
			}
		}
		std::cout << exp_loc.size() << std::endl;

	supply_thresholds = {
		{"basic_opener", 40},
		{"domination_mode",41},
		{"pylon", 8},
		{"geyser", 15},
		{"robotics_facility", 20},
		{"twilight_council", 25},
		{"dark_shrine", 31},
	};

	unit_limits = {
		{"assimilator", 2},
		{"cybernetics_core", 1},
		{"adept", 3},
		{"robotics_facility", 1},
		{"warp_prism", 1},
		{"twilight_council", 1},
		{"dark_shrine", 1},
		{"stargate",1},
	};

	supply_scaling = {
		{"pylon", 8},
		{"gateway", 14},
		{"nexus", 16},
		{"dark_templar", 3}
	};
}

bool BasicSc2Bot::TryBuildPylon()
{
	if (Observation()->GetFoodCap() - Observation()->GetFoodUsed() < supply_scaling["pylon"]
		&& Observation()->GetFoodUsed() <= 200)
	{
		return TryBuildStructure(ABILITY_ID::BUILD_PYLON, UNIT_TYPEID::PROTOSS_PROBE, UNIT_TYPEID::PROTOSS_PYLON);
	}

	return false;
}

bool BasicSc2Bot::TryBuildGeyser()
{
	if (Observation()->GetFoodWorkers() > supply_thresholds["geyser"]
		&& CountUnitType(UNIT_TYPEID::PROTOSS_ASSIMILATOR) < unit_limits["assimilator"] * CountUnitType(UNIT_TYPEID::PROTOSS_NEXUS))
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
	if (nexus_count == 0 || (float)observation->GetFoodUsed() / (float)nexus_count > supply_scaling["nexus"] + (3 * nexus_count) || observation->GetMinerals() > 1200)
	{
		return TryBuildStructure(ABILITY_ID::BUILD_NEXUS, UNIT_TYPEID::PROTOSS_PROBE, UNIT_TYPEID::PROTOSS_NEXUS);
	}
	return false;
}

bool BasicSc2Bot::TryBuildCyber()
{
	if (CountUnitType(UNIT_TYPEID::PROTOSS_GATEWAY) > 0
		&& CountUnitType(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) < unit_limits["cybernetics_core"])
	{
		return TryBuildStructure(ABILITY_ID::BUILD_CYBERNETICSCORE, UNIT_TYPEID::PROTOSS_PROBE, UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
	}

	return false;
}

bool BasicSc2Bot::TryBuildTwilight()
{
	if (Observation()->GetFoodUsed() > supply_thresholds["twilight_council"]
	    && CountUnitType(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) > 0
		&& CountUnitType(UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL) < unit_limits["twilight_council"])
	{
		return TryBuildStructure(ABILITY_ID::BUILD_TWILIGHTCOUNCIL, UNIT_TYPEID::PROTOSS_PROBE, UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL);
	}

	return false;
}

bool BasicSc2Bot::TryBuildStargate()
{
	//build cyber core first, otherwise if we are under limit build a stargate 
	if (CountUnitType(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) == 0){
		TryBuildCyber();
		return false;
	}
	else if (CountUnitType(UNIT_TYPEID::PROTOSS_STARGATE) < unit_limits["stargate"]){
		return TryBuildStructure(ABILITY_ID::BUILD_STARGATE, UNIT_TYPEID::PROTOSS_PROBE, UNIT_TYPEID::PROTOSS_STARGATE);
	}
	return false;
}

bool BasicSc2Bot::TryBuildDarkshrine()
{
	if (Observation()->GetFoodUsed() > supply_thresholds["dark_shrine"]
	    && CountUnitType(UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL) > 0
		&& CountUnitType(UNIT_TYPEID::PROTOSS_DARKSHRINE) < unit_limits["dark_shrine"])
	{
		return TryBuildStructure(ABILITY_ID::BUILD_DARKSHRINE, UNIT_TYPEID::PROTOSS_PROBE, UNIT_TYPEID::PROTOSS_DARKSHRINE);
	}

	return false;
}

bool BasicSc2Bot::TryBuildGateway()
{
	if (CountUnitType(UNIT_TYPEID::PROTOSS_GATEWAY) < Observation()->GetFoodUsed() / supply_scaling["gateway"])
	{
		return TryBuildStructure(ABILITY_ID::BUILD_GATEWAY, UNIT_TYPEID::PROTOSS_PROBE, UNIT_TYPEID::PROTOSS_GATEWAY);
	}

	return false;
}

bool BasicSc2Bot::TryBuildRoboticsFacility()
{
	if (Observation()->GetFoodUsed() > supply_thresholds["robotics_facility"]
		&& CountUnitType(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY) < unit_limits["robotics_facility"])
	{
		return TryBuildStructure(ABILITY_ID::BUILD_ROBOTICSFACILITY, UNIT_TYPEID::PROTOSS_PROBE, UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
	}

	return false;
}

bool BasicSc2Bot::InBasicOpener()
{
	if (Observation()->GetFoodUsed() < supply_thresholds["basic_opener"])
	{
		return true;
	}
	return false;
}

bool BasicSc2Bot::InDominationMode()
{
	//if we are later in the game and no enemies, then we must have wiped the main ones out
	if (Observation()->GetFoodUsed() > supply_thresholds["domination_mode"] && 
	Observation()->GetUnits(Unit::Alliance::Enemy).empty())
	{
		dom_mode = true;
		return true;
	}
	//allow switch off of dom mode in case we get hit hard
	dom_mode = false;
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

		case UNIT_TYPEID::PROTOSS_NEXUS: {			
			// trains workers until full.
			// note, we need to update unit->assigned_harvesters because currently it counts scouting probes and dead probes.
			if (unit->assigned_harvesters < (static_cast<size_t>(unit->ideal_harvesters) + 6) * CountUnitType(UNIT_TYPEID::PROTOSS_NEXUS))
			{
				//Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_PROBE);
			}

			break;
		}
		case UNIT_TYPEID::PROTOSS_STARGATE: {
			Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_VOIDRAY);
			break;
		}
		case UNIT_TYPEID::PROTOSS_VOIDRAY: {
			scouting_system.SendScout(unit,dom_mode);
			break;
		}
		case UNIT_TYPEID::PROTOSS_GATEWAY: {
			OnGatewayIdle(unit);
			break;
		}
		
		case UNIT_TYPEID::PROTOSS_WARPGATE: {
			OnWarpGateIdle(unit);
			break;
		}
									 
		case UNIT_TYPEID::PROTOSS_CYBERNETICSCORE: {
			Actions()->UnitCommand(unit, ABILITY_ID::RESEARCH_WARPGATE);
			break;
		}

		case UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY: {
			OnRoboticsFacilityIdle(unit);
			break;
		}
		case UNIT_TYPEID::PROTOSS_DARKTEMPLAR: {
			scouting_system.SendScout(unit,dom_mode);
			break;
		}

		case UNIT_TYPEID::PROTOSS_WARPPRISM: {
			OnWarpPrismIdle(unit);
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

void BasicSc2Bot::OnGatewayIdle(const Unit* unit) {
	if (CountUnitType(UNIT_TYPEID::PROTOSS_ADEPT) < unit_limits["adept"])
	{
		Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_ADEPT);
	}
}

void BasicSc2Bot::OnWarpGateIdle(const Unit* unit) {
	// get random warp-in position adjustment
	float rx = GetRandomScalar();
	float ry = GetRandomScalar();
	
	// warp in at random power source by default
	int ri = GetRandomInteger(0, Observation()->GetPowerSources().size() - 1);
	PowerSource warp_in_source = Observation()->GetPowerSources()[ri];

	// warp in at prism if found
	Units units = Observation()->GetUnits(Unit::Alliance::Self);
	for (const auto& unit : units) {
		if (unit->unit_type == UNIT_TYPEID::PROTOSS_WARPPRISMPHASING) {
			Tag tag = unit->tag;
			for (const auto& power_source : Observation()->GetPowerSources()) {
				if (power_source.tag == tag) {
					warp_in_source = power_source;
					break;
				}
			}
			break;
		}
	}
	
	if (CountUnitType(UNIT_TYPEID::PROTOSS_DARKTEMPLAR) < Observation()->GetFoodUsed() / supply_scaling["dark_templar"])
	{
		Actions()->UnitCommand(unit,
			ABILITY_ID::TRAINWARP_DARKTEMPLAR,
			Point2D(warp_in_source.position.x + rx * warp_in_source.radius, warp_in_source.position.y + ry * warp_in_source.radius));
	}
}

void BasicSc2Bot::OnRoboticsFacilityIdle(const Unit* unit) {
	if (CountUnitType(UNIT_TYPEID::PROTOSS_WARPPRISM) < unit_limits["warp_prism"]) {
		Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_WARPPRISM);
	}
}

void BasicSc2Bot::OnWarpPrismIdle(const Unit* unit) {
	// moves warp prism to a location offset from the direct path between bases
	if (Point2DI(unit->pos) != Point2DI(warp_in_position)) {
		Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVE, warp_in_position);
	}
	else {
		Actions()->UnitCommand(unit, ABILITY_ID::MORPH_WARPPRISMPHASINGMODE);
	}
}

// Determine where warp prism should move and where units should warp in to.
void BasicSc2Bot::InitWarpInLocation() {
	// specify proportion of distance between bases to travel and distance offset from the direct line
	// between bases
	float distance_factor = 0.8f;
	int normal_distance = 10;

	const GameInfo& game_info = Observation()->GetGameInfo();
	Point2D enemy_base = game_info.enemy_start_locations.front();
	// TODO: verify this is the player start location and see if there's a better way to get that data
	Point2D player_base = game_info.start_locations.back();

	Point2D line_between_bases = enemy_base - player_base;

	// TODO: figure out based on scout probe's path which normal is less likely to run into enemies
	Point2D normal1(-line_between_bases.y, line_between_bases.x);
	Point2D normal2(line_between_bases.y, -line_between_bases.x);
	Normalize2D(normal1);
	Normalize2D(normal2);

	float distance = Distance2D(enemy_base, player_base);
	Normalize2D(line_between_bases);
	Point2D position_between_bases = player_base + line_between_bases * distance * distance_factor;

	Point2D position = position_between_bases + normal1 * normal_distance;

	// TODO: look into more checks for if the location is pathable (Observation()->IsPathable)
	warp_in_position = position;
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
						if (SqDist(poss_nex, poss_geyser) < 70)		// makes sure geyser is close enough to a nexus. Only loops once per gas_id
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
			Units resources;
			resources.push_back(geyser);
			resources.push_back(geyser2);
			float midx = (geyser->pos.x + geyser2->pos.x) / 2.0;
			float midy = (geyser->pos.y + geyser2->pos.y) / 2.0;
			float x, y, dist1, xdif, ydif;
			for (const Unit* possMineral : poss_geysers)
			{
				if (possMineral->mineral_contents)
				{
					xdif = midx - possMineral->pos.x;
					ydif = midy - possMineral->pos.y;
					dist1 = (xdif * xdif) + (ydif * ydif);
					if (dist1 < 90)
					{
						resources.push_back(possMineral);
					}
				}
			}



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
		for (const Unit * unit : observation->GetUnits(Unit::Alliance(1)))
		{
			if (unit->unit_type == UNIT_TYPEID::PROTOSS_WARPPRISM)
			{
				for (const PowerSource& powersource : observation->GetPowerSources())
				{

					if (powersource.tag != unit->tag)
						Actions()->UnitCommand(unit_to_build,
							ability_type_for_structure,
							Point2D(powersource.position.x + rx * powersource.radius, powersource.position.y + ry * powersource.radius));
					return true;
				}
			}
		}
		for (const PowerSource& powersource : observation->GetPowerSources())
		{
			Actions()->UnitCommand(unit_to_build,
				ability_type_for_structure,
				Point2D(powersource.position.x + rx * powersource.radius, powersource.position.y + ry * powersource.radius));
			return true;
		}
		
	}

	return false;

}