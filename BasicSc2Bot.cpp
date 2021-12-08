#include <iostream>
#include <sc2api/sc2_unit_filters.h>

#include "BasicSc2Bot.h"

using namespace sc2;

void BasicSc2Bot::OnGameStart() {
	
	InitData();
	InitWarpInLocation();

	scouting_system.Init(Observation(), Actions(), base_locs);
	defense_system.Init(Observation(), Actions());
	attack_system.Init(Observation(), Actions(), base_locs);
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

	TryBuildGeyser();
	TryBuildExpo();
	TryBuildCyber();
	TryBuildGateway();
	TryBuildTwilightCouncil();
	TryBuildDarkShrine();
	
	if (warp_prism_toggle) {
		TryBuildRoboticsFacility();
	}
	
	//if we have cleared out the map later in the game
	if (InDominationMode()){
		TryBuildStargate();
	}
	return;
}

void BasicSc2Bot::InitData() {
	
	// Create a vector with all possible expansion locations in the map
	expo_spots = search::CalculateExpansionLocations(Observation(), Query());
	
	const GameInfo& game_info = Observation()->GetGameInfo();
	for (const Point2D& sl : game_info.start_locations)
	{
		base_locs.push_back(Point3D(sl.x, sl.y, 1));
	}

	for (const Point3D& exp : expo_spots)
	{
		base_locs.push_back(exp);
	}

	supply_thresholds = {
		{"domination_mode", 41},
		{"pylon", 8},
		{"geyser", 15},
		{"robotics_facility", 20},
		{"twilight_council", 25},
		{"dark_shrine", 31},
	};

	unit_limits = {
		{"cybernetics_core", 1},
		{"adept", 3},
		{"robotics_facility", 1},
		{"warp_prism", 1},
		{"twilight_council", 1},
		{"dark_shrine", 1},
		{"stargate", 1},
		{"probe", 100}
	};

	supply_scaling = {
		{"pylon", 8},
		{"gateway", 14},
		{"nexus", 16},
		{"dark_templar", 3}
	};

	mineral_counts = {
		{"expo", 1200},
		{"gateway", 1000},
		{"dark_templar", 800}
	};

	sq_distances = {
		{"geyser", 90},
	};
}

bool BasicSc2Bot::TryBuildPylon()
{
	if (Observation()->GetMinerals() >= 100 && Observation()->GetFoodCap() - Observation()->GetFoodUsed() < supply_scaling["pylon"]
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
	bool enough_workers = Observation()->GetFoodWorkers() > supply_thresholds["geyser"];
	bool enough_minerals = Observation()->GetMinerals() >= 75;
	bool geyser_nexus_ratio = CountUnitType(UNIT_TYPEID::PROTOSS_ASSIMILATOR) < 1 + (2 * CountUnitType(UNIT_TYPEID::PROTOSS_NEXUS));
	
	if (enough_workers && enough_minerals && geyser_nexus_ratio)
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
	bool enough_minerals = observation->GetMinerals() >= 400;
	bool food_nexus_ratio = (nexus_count == 0) || ((float)observation->GetFoodUsed() / (float)nexus_count) > (supply_scaling["nexus"] + (3 * nexus_count));
	bool high_minerals = observation->GetMinerals() > mineral_counts["expo"]; 
	
	if (!no_more_expos && ( food_nexus_ratio || high_minerals))
	{
		expanding = true;
		if (enough_minerals) 
		{

			return TryBuildStructure(ABILITY_ID::BUILD_NEXUS, UNIT_TYPEID::PROTOSS_PROBE, UNIT_TYPEID::PROTOSS_NEXUS);
		}

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

bool BasicSc2Bot::TryBuildTwilightCouncil()
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
	if (CountUnitType(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) == 0) {
		TryBuildCyber();
		return false;
	}
	else if (CountUnitType(UNIT_TYPEID::PROTOSS_STARGATE) < unit_limits["stargate"]){
		return TryBuildStructure(ABILITY_ID::BUILD_STARGATE, UNIT_TYPEID::PROTOSS_PROBE, UNIT_TYPEID::PROTOSS_STARGATE);
	}
	return false;
}

bool BasicSc2Bot::TryBuildDarkShrine()
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
	int gateway_count = CountUnitType(UNIT_TYPEID::PROTOSS_GATEWAY);
	int nexus_count = CountUnitType(UNIT_TYPEID::PROTOSS_NEXUS);

	bool food_gateway_ratio = gateway_count < Observation()->GetFoodUsed() / (supply_scaling["gateway"] + (4 * gateway_count));
	bool gateway_nexus_ratio = gateway_count < 2 * nexus_count;
	bool high_minerals = Observation()->GetVespene() > mineral_counts["gateway"] && Observation()->GetMinerals() > mineral_counts["gateway"];

	if ((!expanding && food_gateway_ratio && gateway_nexus_ratio) || high_minerals)
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

bool BasicSc2Bot::InDominationMode()
{
	//if we are later in the game and no enemies, then we must have wiped the main ones out
	if (Observation()->GetFoodUsed() > supply_thresholds["domination_mode"]
		&& Observation()->GetUnits(Unit::Alliance::Enemy).empty())
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
		if (u->mineral_contents) {
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
		
		// check if unit is harvesting at something other than a geyser
		bool is_harvesting = false;
		bool not_at_geyser = false;
		if (!unit->orders.empty()) {
			is_harvesting = unit->orders[0].ability_id == ABILITY_ID::HARVEST_GATHER;
			
			const Unit* target_unit = observation->GetUnit(unit->orders[0].target_unit_tag);
			if (target_unit) {
				not_at_geyser = target_unit->unit_type != UNIT_TYPEID::PROTOSS_ASSIMILATOR;
			}
		}

		if (unit->orders.empty() || (is_harvesting && not_at_geyser))
		{
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
			bool under_probe_limit = CountUnitType(UNIT_TYPEID::PROTOSS_PROBE) < unit_limits["probe"];
			if (unit->ideal_harvesters > unit->assigned_harvesters && unit->orders.empty() && under_probe_limit)
			{
				Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_PROBE);
			}
		}
	}
	return true;
}

void BasicSc2Bot::OnUnitIdle(const Unit* unit) {
	switch (unit->unit_type.ToType()) {

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

	// early-out if there are no valid power sources
	if (Observation()->GetPowerSources().size() == 0) {
		return;
	}
	
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

	bool mineral_excess = Observation()->GetMinerals() > mineral_counts["dark_templar"];
	
	if (mineral_excess || (!expanding && CountUnitType(UNIT_TYPEID::PROTOSS_DARKTEMPLAR) < Observation()->GetFoodUsed() / supply_scaling["dark_templar"]))
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

bool BasicSc2Bot::TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type, UNIT_TYPEID structure_type) {
	const ObservationInterface* observation = Observation();
	const Unit* scout = scouting_system.GetScout();

	// If a unit already is building a supply structure of this type, do nothing.
	// Also get a probe to build the structure.
	const Unit* unit_to_build = nullptr;
	Units units = observation->GetUnits(Unit::Alliance::Self, IsUnit(unit_type));
	Units buildings = observation->GetUnits(Unit::Alliance::Self, IsUnit(structure_type));
	if (structure_type != UNIT_TYPEID::PROTOSS_ASSIMILATOR)
	{
		for (const auto& unit : buildings)
		{	// checks if we are already building a structure of this type.
			if (unit->build_progress < 1.0)
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
		Units poss_geysers = observation->GetUnits(Unit::Alliance::Neutral);
		Units nexi = observation->GetUnits(Unit::Alliance(Unit::Alliance::Self), IsUnit(UNIT_TYPEID::PROTOSS_NEXUS));

		for (const auto& poss_geyser : poss_geysers)
		{
			if (poss_geyser->vespene_contents)					// makes sure target is a geyser
			{
				bool close_to_nexus = false;
				for (const auto& nexus : nexi)
				{
					if (SqDist(nexus, poss_geyser) < sq_distances["geyser"])		// makes sure geyser is close enough to a nexus. Only loops once per gas_id
					{
						close_to_nexus = true;
					}
				}
				if (close_to_nexus && Query()->Placement(ability_type_for_structure, poss_geyser->pos))									// assigns the build task if we are near a nexus.
				{
					Actions()->UnitCommand(unit_to_build,
						ability_type_for_structure,
						poss_geyser);
					return true;
				}
			}
		}
		return false;
	}
	else if (structure_type == UNIT_TYPEID::PROTOSS_NEXUS)
	{
		std::vector<Point3D> validExpos;
		for (Point3D expoSpot : expo_spots)
		{
			if (Query()->Placement(ability_type_for_structure, Point2D(expoSpot.x, expoSpot.y)))
			{
				validExpos.push_back(expoSpot);
			}
		}

		if (validExpos.empty())
		{
			no_more_expos = true;
			expanding = false;
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
		expanding = false;
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
		}
		
	}

	return false;

}