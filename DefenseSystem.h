#pragma once

#include "sc2api/sc2_api.h"

using namespace sc2;

class DefenseSystem {
public:
	void Init(const ObservationInterface* obs, ActionInterface* act);
	void DefenseStep();
	void SetDefense();
	void SendDefense();
	size_t DefenseSystem::CountUnitType(UNIT_TYPEID unit_type);

private:
	Units defense;
	const ObservationInterface* observation = nullptr;
	ActionInterface* actions = nullptr;
};