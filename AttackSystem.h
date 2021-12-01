#pragma once

#include "sc2api/sc2_api.h"

using namespace sc2;

class AttackSystem {
public:
	void Init(const ObservationInterface* obs, ActionInterface* act);
	void AttackStep();
	void SetAttackUnits();
	void SendAttackUnits();
	void SetTarget();
	size_t CountUnitType(UNIT_TYPEID unit_type);

private:
	const ObservationInterface* observation = nullptr;
	ActionInterface* actions = nullptr;

	Units attack_units;
	Point2D target;
};
