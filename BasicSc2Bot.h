#ifndef BASIC_SC2_BOT_H_
#define BASIC_SC2_BOT_H_

#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"

#include "ScoutingSystem.h"
#include "DefenseSystem.h"
#include "AttackSystem.h"
#include "SlanderSystem.h"

using namespace sc2;

class BasicSc2Bot : public sc2::Agent {
public:
	virtual void OnGameStart() final;
	virtual void OnGameEnd() final;
	virtual void OnStep() final;
	virtual void OnUnitIdle(const Unit* unit) final;
	bool TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type, UNIT_TYPEID structure_type);
	const Unit* FindNearestMineralPatch(const Point2D& start);
	size_t CountUnitType(UNIT_TYPEID unit_type);
	bool InBasicOpener();
	bool AssignProbeToGas(const Unit *geyser);
	float SqDist(const Unit *a, const Unit *b) const;
	float SqDist(const Point3D& a, const Point3D& b) const;
	bool CheckHarvesterStatus();

	bool TryBuildPylon();

	// early game functions
	bool TryBuildGeyser();
	bool TryBuildExpo();
	bool TryBuildCyber();
	bool TryBuildGateway();
	bool TryBuildRoboticsFacility();
	bool TryBuildTwilightCouncil();
	bool TryBuildDarkShrine();

	void OnGatewayIdle(const Unit* unit);
	void OnWarpGateIdle(const Unit* unit);
	void OnRoboticsFacilityIdle(const Unit* unit);
	void OnWarpPrismIdle(const Unit* unit);
	void InitWarpInLocation();
	bool InDominationMode();
	bool TryBuildStargate();
	void InitData();
	bool AssignBuildOrder(const Unit* probe);
	bool AssignBuilder();


private:
	ScoutingSystem scouting_system;
	DefenseSystem defense_system;
	AttackSystem attack_system;
	SlanderSystem slander_system;
	
	// all possible base locations
	std::vector<Point3D> base_locs;
	
	// all possible expansion locations
	std::vector<Point3D> expo_spots;
	
	Point2D warp_in_position;
	bool dom_mode = false;
	bool expanding = false;
	bool no_more_expos = false;
	
	std::map<std::string, int> supply_thresholds;
	std::map<std::string, int> unit_limits;
	std::map<std::string, int> supply_scaling;
	std::map<std::string, int> mineral_counts;
	std::map<std::string, int> sq_distances;
	std::queue<ABILITY_ID> ability_build_queue;
	std::queue<UNIT_TYPEID> building_build_queue;
	std::queue<Point3D> pos_build_queue;
	//std::queue<tag> geyser_build_queue;

	// Set to true to enable warp prism warp in
	bool warp_prism_toggle = false;
	const Unit* builder;
};

#endif
