#ifndef BASIC_SC2_BOT_H_
#define BASIC_SC2_BOT_H_

#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"

using namespace sc2;

class BasicSc2Bot : public sc2::Agent {
public:
	virtual void OnGameStart() final;
	virtual void OnStep() final;
	virtual void OnUnitIdle(const Unit* unit) final;
	bool TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type = UNIT_TYPEID::PROTOSS_PROBE);
	bool TryBuildSupplyDepot();
	const Unit* FindNearestMineralPatch(const Point2D& start);
	size_t CountUnitType(UNIT_TYPEID unit_type);
	bool TryBuildBarracks();
	bool InBasicOpener(int food_used) const;



	// early game functions
	bool TryBuildAdept();
	bool TryBuildWallPylon();
	bool TryBuildGeyser();
	bool TryBuildExpo();
	bool TryBuildCyber();
	bool TryBuildFirstGateway();
	bool TryBuildCliffPylon();



private:
};

#endif