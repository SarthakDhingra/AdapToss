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


// for statistics
void BasicSc2Bot::OnGameEnd() {
	
	// get results
	std::vector<PlayerResult> results = Observation()->GetResults();
	std::string result;

	// print our id
	std::cout << "Player id: " << Observation()->GetPlayerID() << std::endl;

	// parse different result cases
	for (int i = 0; i < results.size(); ++i) {
		switch (results[i].result) {
			case GameResult::Win:
				result = "Win";
				break;
			case GameResult::Loss:
				result = "Loss";
				break;
			case GameResult::Tie:
				result = "Tie";
				break;
			default:
				result = "Undecided";
				break;
		}

		// print player results
		std::cout << "Player " << results[i].player_id << " result: " << result << std::endl;
		
	}
	
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
	TryBuildGeyser();
	TryBuildExpo();
	TryBuildCyber();
	TryBuildGateway();
	TryBuildTwilight();
	TryBuildDarkshrine();
	//TryBuildRoboticsFacility();
	
	//if we have cleared out the map later in the game
	if (InDominationMode()){
		TryBuildStargate();
	}
	return;
}

void BasicSc2Bot::InitData() {
	//how close we get to the base with DTs
	approach_increment = 2.0;
	
	//find all neutral units (includes resources), go through them to find locations of gases
	//these cover the mains and the expansions
	Units poss_geysers = Observation()->GetUnits(Unit::Alliance::Neutral);		// gets neutral units
	const GameInfo& game_info = Observation()->GetGameInfo();
	for (Point2D sl : game_info.start_locations)
	{
		exp_loc.push_back(Point3D(sl.x, sl.y, 1));
	}

	for (Point3D exp : search::CalculateExpansionLocations(Observation(), Query()))
	{
		exp_loc.push_back(exp);
	}

	supply_thresholds = {
		{"basic_opener", 40},
		{"domination_mode", 41},
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

	mineral_counts = {
		{"geyser", 75},
		{"expo", 1200},
		{"gateway", 1700}
	};

	sq_distances = {
		{"geyser", 70},
	};
}

bool BasicSc2Bot::TryBuildPylon()
{
	if (Observation()->GetFoodCap() - Observation()->GetFoodUsed() < supply_scaling["pylon"]
		&& Observation()->GetFoodCap() <= 200)
	{
		// check for building gateway before more pylons
		if (CountUnitType(UNIT_TYPEID::PROTOSS_PYLON) < 1 || (CountUnitType(UNIT_TYPEID::PROTOSS_GATEWAY) + CountUnitType(UNIT_TYPEID::PROTOSS_WARPGATE)))
		{
			return TryBuildStructure(ABILITY_ID::BUILD_PYLON, UNIT_TYPEID::PROTOSS_PROBE, UNIT_TYPEID::PROTOSS_PYLON);
		}
	}

	return false;
}

bool BasicSc2Bot::TryBuildGeyser()
{
	if (Observation()->GetFoodWorkers() > supply_thresholds["geyser"] && Observation()->GetMinerals() >= mineral_counts["geyser"]
		&& CountUnitType(UNIT_TYPEID::PROTOSS_ASSIMILATOR) < 1 + (2 * CountUnitType(UNIT_TYPEID::PROTOSS_NEXUS)))
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
	if (observation->GetMinerals() >= 400 && 
		(nexus_count == 0 || ((float) observation->GetFoodUsed() / (float) nexus_count) > (supply_scaling["nexus"] + (3 * nexus_count)) || observation->GetMinerals() > mineral_counts["expo"]))
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
	if (CountUnitType(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) > 0
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
	if (CountUnitType(UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL) > 0
		&& CountUnitType(UNIT_TYPEID::PROTOSS_DARKSHRINE) < unit_limits["dark_shrine"])
	{
		return TryBuildStructure(ABILITY_ID::BUILD_DARKSHRINE, UNIT_TYPEID::PROTOSS_PROBE, UNIT_TYPEID::PROTOSS_DARKSHRINE);
	}

	return false;
}

bool BasicSc2Bot::TryBuildGateway()
{
	// Ties gateway count to supply used and nexus count, or builds many if we have too much money
	if ((CountUnitType(UNIT_TYPEID::PROTOSS_GATEWAY) < Observation()->GetFoodUsed() / (supply_scaling["gateway"] + (4 * CountUnitType(UNIT_TYPEID::PROTOSS_GATEWAY))) 
		&& CountUnitType(UNIT_TYPEID::PROTOSS_GATEWAY) < 2 * CountUnitType(UNIT_TYPEID::PROTOSS_NEXUS))
		|| Observation()->GetMinerals() > mineral_counts["gateway"])
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

// Returns closest mineral patch to supplied location
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
	Units units = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::PROTOSS_PROBE));
	for (const auto& unit : units) {
		if (unit->orders.empty() || (unit->orders[0].ability_id == ABILITY_ID::HARVEST_GATHER
			&& observation->GetUnit(unit->orders[0].target_unit_tag)->unit_type != UNIT_TYPEID::PROTOSS_ASSIMILATOR)) {
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


// returns the squared distance between two Units
float BasicSc2Bot::SqDist(const Unit* a, const Unit* b) const
{
	return SqDist(a->pos, b->pos);
}

// returns the squared distance between two Units
float BasicSc2Bot::SqDist(const Point3D& a, const Point3D& b) const
{
	float x, y;
	x = (a.x - b.x);
	x *= x;
	y = (a.y - b.y);
	y *= y;
	return x + y;
}

bool BasicSc2Bot::TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type) {
	const ObservationInterface* observation = Observation();
	const Unit* scout = scouting_system.GetScout();

	// If a unit already is building a supply structure of this type, do nothing.
	// Also get an scv to build the structure.
	const Unit* unit_to_build = nullptr;
	Units units = observation->GetUnits(Unit::Alliance::Self, IsUnit(unit_type));
	for (const auto& unit : units) {
		for (const auto& order : unit->orders) {
			if (order.ability_id == ability_type_for_structure) {
				return false;
			}
		}

		if (unit->tag != scout->tag) {
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
	const Unit* scout = scouting_system.GetScout();

	// If a unit already is building a supply structure of this type, do nothing.
	// Also get a probe to build the structure.
	const Unit* unit_to_build = nullptr;
	Units units = observation->GetUnits(Unit::Alliance::Self, IsUnit(unit_type));
	if (structure_type != UNIT_TYPEID::PROTOSS_ASSIMILATOR)
	{
		for (const auto& unit : units)
		{	// checks if we are already building a structure of this type.
			if (unit->unit_type == structure_type && unit->build_progress < 1.0)
			{
				return false;
			}
		}
	}

	
	for (const auto& unit : units) {
		for (const auto& order : unit->orders) {
			if (order.ability_id == ability_type_for_structure && unit->is_alive) {
				return false;
			}
		}


		if (unit->tag != scout->tag && unit->is_alive) {
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
		Units poss_geysers = observation->GetUnits(Unit::Alliance(Unit::Alliance::Neutral));
		Units nexi = observation->GetUnits(Unit::Alliance(Unit::Alliance::Self), IsUnit(UNIT_TYPEID::PROTOSS_NEXUS));

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
					for (const auto& nexus : nexi)
					{
						if (SqDist(nexus, poss_geyser) < sq_distances["geyser"])		// makes sure geyser is close enough to a nexus. Only loops once per gas_id
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
		QueryInterface* query = Query();
		std::vector<Point3D> expoSpots = search::CalculateExpansionLocations(observation, Query());
		std::vector<Point3D> validExpos;
		for (Point3D expoSpot : expoSpots)
		{
			if (Query()->Placement(ability_type_for_structure, Point2D(expoSpot.x, expoSpot.y)))
			{
				validExpos.push_back(expoSpot);
			}
		}

		if (validExpos.empty())
		{
			return false;
		}

		Point3D closestPoint = validExpos.front();
		float closestDist = SqDist(unit_to_build->pos, closestPoint);
		float potentialDist;

		for (Point3D expo : validExpos)
		{
			potentialDist = SqDist(unit_to_build->pos, expo);
			if (potentialDist < closestDist)
			{
				closestDist = potentialDist;
				closestPoint = expo;
			}
		}

		observation->GetUnits(Unit::Alliance(Unit::Alliance::Self), IsUnit(unit_to_build->unit_type));
		Actions()->UnitCommand(unit_to_build,
			ability_type_for_structure,
			closestPoint);
		return true;
	}
	else
	{
		for (const Unit * unit : observation->GetUnits(Unit::Alliance(Unit::Alliance::Self)))
		{
			if (unit->unit_type == UNIT_TYPEID::PROTOSS_WARPPRISM)				// checks to not build at a warp prism
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