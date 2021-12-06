#pragma once

#include "sc2api/sc2_api.h"

using namespace sc2;

class AttackSystem {
public:
	void Init(const ObservationInterface* obs, ActionInterface* act,std::vector<Point3D> locs);
	void AttackStep();
	void SetAttackUnits();
	void SendAttackUnits();
	Point3D GetTarget(const Unit * unit);
	size_t CountUnitType(UNIT_TYPEID unit_type);

private:
	const ObservationInterface* observation = nullptr;
	ActionInterface* actions = nullptr;
	std::vector<Point3D> exp_locs;
	Units attack_units;
};
