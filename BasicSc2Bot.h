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

using namespace sc2;

class BasicSc2Bot : public sc2::Agent {
public:
	virtual void OnGameStart() final;
	virtual void OnStep() final;
	virtual void OnUnitIdle(const Unit* unit) final;
	bool TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type = UNIT_TYPEID::PROTOSS_PROBE);
	bool TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type, UNIT_TYPEID structure_type);
	const Unit* FindNearestMineralPatch(const Point2D& start);
	size_t CountUnitType(UNIT_TYPEID unit_type);
	bool InBasicOpener();
	bool AssignProbeToGas(const Unit *geyser);
	float SqDist(const Unit *a, const Unit *b);
	bool CheckHarvesterStatus();

	bool TryBuildPylon();

	// early game functions
	bool TryBuildGeyser();
	bool TryBuildExpo();
	bool TryBuildCyber();
	bool TryBuildGateway();
	bool TryBuildRoboticsFacility();
	bool TryBuildTwilight();
	bool TryBuildDarkshrine();

	void OnGatewayIdle(const Unit* unit);
	void OnWarpGateIdle(const Unit* unit);
	void OnRoboticsFacilityIdle(const Unit* unit);
	void OnWarpPrismIdle(const Unit* unit);
	void InitWarpInLocation();
	bool InDominationMode();
	bool TryBuildStargate();
	void InitData();


private:
	ScoutingSystem scouting_system;
	DefenseSystem defense_system;
	AttackSystem attack_system;
	
	Point2D warp_in_position;
	bool dom_mode = false;
	double approach_increment;
	std::map<std::string, int> supply_thresholds;
	std::map<std::string, int> unit_limits;
	std::map<std::string, int> supply_scaling;
};

#endif
