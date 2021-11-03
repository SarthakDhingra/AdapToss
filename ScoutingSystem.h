#pragma once

#include "sc2api/sc2_api.h"

using namespace sc2;

class ScoutingSystem {
public:
	void Initialize(const ObservationInterface* obs, ActionInterface* act);
	void ScoutingStep();

	void SetScout();
	void SendScout();

private:
	const ObservationInterface* observation = nullptr;
	ActionInterface* actions = nullptr;
	
	const Unit* scout = nullptr;
	//UNIT_TYPEID scout_type = UNIT_TYPEID::PROTOSS_PROBE;
	UNIT_TYPEID scout_type = UNIT_TYPEID::TERRAN_SCV;
};

