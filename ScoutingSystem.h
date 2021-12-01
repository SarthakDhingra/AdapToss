#pragma once

#include "sc2api/sc2_api.h"

using namespace sc2;

class ScoutingSystem {
public:
	void Init(const ObservationInterface* obs, ActionInterface* act);
	void InitScoutingData();
	
	void ScoutingStep();

	void SetScout();
	void SendScout(const Unit * unit = nullptr, bool dom = false);
	void ScoutEarlyRush();
	void ScoutDetection();

private:
	const ObservationInterface* observation = nullptr;
	ActionInterface* actions = nullptr;
	
	const Unit* scout = nullptr;
	UNIT_TYPEID scout_type = UNIT_TYPEID::PROTOSS_PROBE;
	Race enemy_race;

	int early_game_supply;
	//places to scout
	std::vector<Point3D> exp_loc;
	int pos = 0;
	// Keep track of data that's meaningful for adjusting our strategy
	std::map<std::string, bool> scouting_data;

	// Keep track of what the supply threshold is for some early decisions
	std::map<std::string, int> early_scouting_thresholds;
};

